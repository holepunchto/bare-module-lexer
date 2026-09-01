#include <assert.h>
#include <bare.h>
#include <js.h>
#include <stdbool.h>
#include <stdint.h>
#include <uv.h>

#include "lib/lex.h"

static int
bare_module_lexer__get_buffer_info(js_env_t *env, js_value_t *buffer, void **data, size_t *len) {
  int err;

  bool is_arraybuffer;
  err = js_is_arraybuffer(env, buffer, &is_arraybuffer);
  assert(err == 0);

  if (is_arraybuffer) {
    err = js_get_arraybuffer_info(env, buffer, data, len);
    assert(err == 0);

    return 0;
  }

  bool is_sharedarraybuffer;
  err = js_is_sharedarraybuffer(env, buffer, &is_sharedarraybuffer);
  assert(err == 0);

  if (is_sharedarraybuffer) {
    err = js_get_sharedarraybuffer_info(env, buffer, data, len);
    assert(err == 0);

    return 0;
  }

  return -1;
}

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

static js_value_t *
bare_module_lexer_lex(js_env_t *env, js_callback_info_t *info) {
  int err;

  size_t argc = 3;
  js_value_t *argv[3];

  err = js_get_callback_info(env, info, &argc, argv, NULL, NULL);
  assert(err == 0);

  void *data;
  size_t byte_len;
  if (bare_module_lexer__get_buffer_info(env, argv[0], &data, &byte_len) < 0) {
    err = js_throw_type_error(env, NULL, "Input must be a buffer");
    assert(err == 0);

    return NULL;
  }

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

  utf8_t *input = (utf8_t *) data + offset;

  js_value_t *imports;
  err = js_create_array(env, &imports);
  assert(err == 0);

  js_value_t *exports;
  err = js_create_array(env, &exports);
  assert(err == 0);

  err = bare_module_lexer__lex(env, imports, exports, input, (size_t) len);
  if (err < 0) return NULL;

  js_value_t *result;
  err = js_create_object(env, &result);
  assert(err == 0);

  err = js_set_named_property(env, result, "imports", imports);
  if (err < 0) return NULL;

  err = js_set_named_property(env, result, "exports", exports);
  if (err < 0) return NULL;

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
