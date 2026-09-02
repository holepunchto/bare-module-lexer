#include <assert.h>
#include <bare.h>
#include <js.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <uv.h>

#include "lib/lex.h"

static int
bare_module_lexer__get_int64(js_env_t *env, js_value_t *value, int64_t *result) {
  int err;

  bool is_number;
  err = js_is_number(env, value, &is_number);
  assert(err == 0);

  if (!is_number) return -1;

  err = js_get_value_int64(env, value, result);
  assert(err == 0);

  return 0;
}

// The strings of every range the lexer recorded, laid out as one allocation of
// four sections: an import specifier per import, a name per export, a name per
// import name, and a key and a value per import attribute.
// Whatever fits inline in the context fits inline here too.
#define BARE_MODULE_LEXER__INLINE_STRINGS \
  (BARE_MODULE_LEXER__INLINE_IMPORTS + \
   BARE_MODULE_LEXER__INLINE_EXPORTS + \
   BARE_MODULE_LEXER__INLINE_NAMES + \
   BARE_MODULE_LEXER__INLINE_ATTRIBUTES * 2)

typedef struct {
  js_value_t **specifiers;
  js_value_t **export_names;
  js_value_t **names;
  js_value_t **attributes;

  js_value_t **data;
  js_value_t *data_inline[BARE_MODULE_LEXER__INLINE_STRINGS];
} bare_module_lexer_strings_t;

// Turn every recorded range into a string. This is the last thing to read the
// borrowed input, and so the last thing that may not run JavaScript.
static int
bare_module_lexer__create_strings(js_env_t *env, const bare_module_lexer_t *ctx, const utf8_t *s, bare_module_lexer_strings_t *result) {
  int err;

  size_t len = (size_t) ctx->imports_len +
               (size_t) ctx->exports_len +
               (size_t) ctx->names_len +
               (size_t) ctx->attributes_len * 2;

  js_value_t **data;

  if (len <= BARE_MODULE_LEXER__INLINE_STRINGS) {
    data = result->data_inline;
  } else {
    data = malloc(len * sizeof(js_value_t *));

    if (data == NULL) return -1;
  }

  result->data = data;
  result->specifiers = data;
  result->export_names = result->specifiers + ctx->imports_len;
  result->names = result->export_names + ctx->exports_len;
  result->attributes = result->names + ctx->names_len;

  for (uint32_t i = 0; i < ctx->imports_len; i++) {
    const bare_module_lexer_range_t *range = &ctx->imports[i].specifier;

    err = js_create_string_utf8(env, &s[range->start], range->end - range->start, &result->specifiers[i]);
    if (err < 0) return err;
  }

  for (uint32_t i = 0; i < ctx->exports_len; i++) {
    const bare_module_lexer_range_t *range = &ctx->exports[i].name;

    err = js_create_string_utf8(env, &s[range->start], range->end - range->start, &result->export_names[i]);
    if (err < 0) return err;
  }

  for (uint32_t i = 0; i < ctx->names_len; i++) {
    const bare_module_lexer_name_t *name = &ctx->names[i];

    switch (name->type) {
    case bare_module_lexer__name_star:
      err = js_create_string_utf8(env, (const utf8_t *) "*", 1, &result->names[i]);
      break;

    case bare_module_lexer__name_default:
      err = js_create_string_utf8(env, (const utf8_t *) "default", 7, &result->names[i]);
      break;

    default:
      err = js_create_string_utf8(env, &s[name->range.start], name->range.end - name->range.start, &result->names[i]);
    }

    if (err < 0) return err;
  }

  for (uint32_t i = 0; i < ctx->attributes_len; i++) {
    const bare_module_lexer_attribute_t *attribute = &ctx->attributes[i];

    err = js_create_property_key_utf8(env, &s[attribute->key.start], attribute->key.end - attribute->key.start, &result->attributes[i * 2]);
    if (err < 0) return err;

    err = js_create_string_utf8(env, &s[attribute->value.start], attribute->value.end - attribute->value.start, &result->attributes[i * 2 + 1]);
    if (err < 0) return err;
  }

  return 0;
}

static int
bare_module_lexer__create_position(js_env_t *env, js_value_t *entry, uint32_t statement, const bare_module_lexer_range_t *range) {
  int err;

  js_value_t *position;
  err = js_create_array_with_length(env, 3, &position);
  assert(err == 0);

#define V(i, n) \
  { \
    js_value_t *val; \
    err = js_create_int64(env, n, &val); \
    assert(err == 0); \
    err = js_set_element(env, position, i, val); \
    if (err < 0) return err; \
  }

  V(0, statement);
  V(1, range->start);
  V(2, range->end);
#undef V

  return js_set_named_property(env, entry, "position", position);
}

// Assemble the result from the strings created above. This runs after the
// borrow is over and takes no input pointer.
static int
bare_module_lexer__create_result(js_env_t *env, const bare_module_lexer_t *ctx, const bare_module_lexer_strings_t *strings, js_value_t **result) {
  int err;

  js_value_t *imports;
  err = js_create_array_with_length(env, ctx->imports_len, &imports);
  assert(err == 0);

  for (uint32_t i = 0; i < ctx->imports_len; i++) {
    const bare_module_lexer_import_t *import = &ctx->imports[i];

    js_value_t *entry;
    err = js_create_object(env, &entry);
    assert(err == 0);

    err = js_set_element(env, imports, i, entry);
    if (err < 0) return err;

    err = js_set_named_property(env, entry, "specifier", strings->specifiers[i]);
    if (err < 0) return err;

    js_value_t *type;
    err = js_create_uint32(env, import->type, &type);
    assert(err == 0);

    err = js_set_named_property(env, entry, "type", type);
    if (err < 0) return err;

    js_value_t *names;
    err = js_create_array_with_length(env, import->names_len, &names);
    assert(err == 0);

    for (uint32_t j = 0; j < import->names_len; j++) {
      err = js_set_element(env, names, j, strings->names[import->names + j]);
      if (err < 0) return err;
    }

    err = js_set_named_property(env, entry, "names", names);
    if (err < 0) return err;

    js_value_t *attributes;
    err = js_create_object(env, &attributes);
    assert(err == 0);

    for (uint32_t j = 0; j < import->attributes_len; j++) {
      uint32_t k = (import->attributes + j) * 2;

      err = js_set_property(env, attributes, strings->attributes[k], strings->attributes[k + 1]);
      if (err < 0) return err;
    }

    err = js_set_named_property(env, entry, "attributes", attributes);
    if (err < 0) return err;

    err = bare_module_lexer__create_position(env, entry, import->statement, &import->specifier);
    if (err < 0) return err;
  }

  js_value_t *exports;
  err = js_create_array_with_length(env, ctx->exports_len, &exports);
  assert(err == 0);

  for (uint32_t i = 0; i < ctx->exports_len; i++) {
    const bare_module_lexer_export_t *export = &ctx->exports[i];

    js_value_t *entry;
    err = js_create_object(env, &entry);
    assert(err == 0);

    err = js_set_element(env, exports, i, entry);
    if (err < 0) return err;

    err = js_set_named_property(env, entry, "name", strings->export_names[i]);
    if (err < 0) return err;

    err = bare_module_lexer__create_position(env, entry, export->statement, &export->name);
    if (err < 0) return err;
  }

  js_value_t *object;
  err = js_create_object(env, &object);
  assert(err == 0);

  err = js_set_named_property(env, object, "imports", imports);
  if (err < 0) return err;

  err = js_set_named_property(env, object, "exports", exports);
  if (err < 0) return err;

  *result = object;

  return 0;
}

static js_value_t *
bare_module_lexer_lex(js_env_t *env, js_callback_info_t *info) {
  int err;

  size_t argc = 3;
  js_value_t *argv[3];

  err = js_get_callback_info(env, info, &argc, argv, NULL, NULL);
  assert(err == 0);

  bool is_arraybuffer;
  err = js_is_arraybuffer(env, argv[0], &is_arraybuffer);
  assert(err == 0);

  if (!is_arraybuffer) {
    err = js_throw_type_error(env, NULL, "Input must be a buffer");
    assert(err == 0);

    return NULL;
  }

  void *data;
  size_t byte_len;
  err = js_get_arraybuffer_info(env, argv[0], &data, &byte_len);
  assert(err == 0);

  int64_t offset;
  if (bare_module_lexer__get_int64(env, argv[1], &offset) < 0) {
    err = js_throw_type_error(env, NULL, "Offset must be a number");
    assert(err == 0);

    return NULL;
  }

  int64_t len;
  if (bare_module_lexer__get_int64(env, argv[2], &len) < 0) {
    err = js_throw_type_error(env, NULL, "Length must be a number");
    assert(err == 0);

    return NULL;
  }

  if (offset < 0 || len < 0 || (uint64_t) offset + (uint64_t) len > byte_len) {
    err = js_throw_range_error(env, NULL, "Offset and length are out of bounds");
    assert(err == 0);

    return NULL;
  }

  if ((uint64_t) len > UINT32_MAX) {
    err = js_throw_range_error(env, NULL, "Length is out of bounds");
    assert(err == 0);

    return NULL;
  }

  utf8_t *input = (utf8_t *) data + offset;

  bare_module_lexer_t ctx;
  bare_module_lexer_init(&ctx);

  bare_module_lexer_strings_t strings = {.data = NULL};

  js_value_t *result = NULL;

  err = bare_module_lexer__lex(&ctx, input, (size_t) len);
  if (err < 0) goto done;

  err = bare_module_lexer__create_strings(env, &ctx, input, &strings);
  if (err < 0) goto done;

  // The borrow ends here, nothing below reads `input`.

  err = bare_module_lexer__create_result(env, &ctx, &strings, &result);

done:
  if (strings.data != strings.data_inline) free(strings.data);

  bare_module_lexer_destroy(&ctx);

  if (err < 0) {
    bool is_pending;
    err = js_is_exception_pending(env, &is_pending);
    assert(err == 0);

    if (!is_pending) {
      err = js_throw_error(env, uv_err_name(UV_ENOMEM), uv_strerror(UV_ENOMEM));
      assert(err == 0);
    }

    return NULL;
  }

  return result;
}

static js_value_t *
bare_module_lexer_exports(js_env_t *env, js_value_t *exports) {
  int err;

#define V(name, fn) \
  { \
    js_value_t *val; \
    err = js_create_function(env, name, -1, fn, NULL, &val); \
    assert(err == 0); \
    err = js_set_named_property(env, exports, name, val); \
    assert(err == 0); \
  }

  V("lex", bare_module_lexer_lex)
#undef V

#define V(name, n) \
  { \
    js_value_t *val; \
    err = js_create_uint32(env, n, &val); \
    assert(err == 0); \
    err = js_set_named_property(env, exports, name, val); \
    assert(err == 0); \
  }

  V("REQUIRE", bare_module_lexer_require)
  V("IMPORT", bare_module_lexer_import)
  V("DYNAMIC", bare_module_lexer_dynamic)
  V("ADDON", bare_module_lexer_addon)
  V("ASSET", bare_module_lexer_asset)
  V("RESOLVE", bare_module_lexer_resolve)
  V("REEXPORT", bare_module_lexer_reexport)
#undef V

  return exports;
}

BARE_MODULE(bare_module_lexer, bare_module_lexer_exports)
