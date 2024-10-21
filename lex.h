#ifndef BARE_MODULE_LEXER_H
#define BARE_MODULE_LEXER_H

#include <js.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <utf.h>

enum {
  bare_module_lexer_require = 0x1,
  bare_module_lexer_import = 0x2,
  bare_module_lexer_addon = 0x4,
  bare_module_lexer_asset = 0x8,
  bare_module_lexer_reexport = 0x10,
};

static inline int
bare_module_lexer__add_import (js_env_t *env, js_value_t *imports, uint32_t i, const utf8_t *specifier, size_t len, int type) {
  int err;

  js_value_t *entry;
  err = js_create_object(env, &entry);
  assert(err == 0);

#define V(key, fn, ...) \
  { \
    js_value_t *val; \
    err = fn(env, __VA_ARGS__, &val); \
    assert(err == 0); \
    err = js_set_named_property(env, entry, key, val); \
    assert(err == 0); \
  }

  V("specifier", js_create_string_utf8, specifier, len);
  V("type", js_create_uint32, type);
#undef V

  err = js_set_element(env, imports, i, entry);
  assert(err == 0);

  return 0;
}

static inline int
bare_module_lexer__add_export (js_env_t *env, js_value_t *exports, uint32_t i, const utf8_t *name, size_t len) {
  int err;

  js_value_t *entry;
  err = js_create_object(env, &entry);
  assert(err == 0);

#define V(key, fn, ...) \
  { \
    js_value_t *val; \
    err = fn(env, __VA_ARGS__, &val); \
    assert(err == 0); \
    err = js_set_named_property(env, entry, key, val); \
    assert(err == 0); \
  }

  V("name", js_create_string_utf8, name, len);
#undef V

  err = js_set_element(env, exports, i, entry);
  assert(err == 0);

  return 0;
}

static inline int
bare_module_lexer__lex (js_env_t *env, js_value_t *imports, js_value_t *exports, const utf8_t *s, size_t n) {
  int err;

  uint32_t p = 0;
  uint32_t q = 0;

  size_t i = 0;
  size_t j;
  size_t k;

  int type = 0;

// Current character, unchecked
#define u(offset) (s[i + offset])

// Current character, checked
#define c(offset) (i + offset < n ? u(offset) : -1)

// Whitespace character
#define ws(c) (c == ' ' || c == '\t' || c == 0xb || c == 0xc || c == 0xa0)

// Line terminator
#define lt(c) (c == 0xa || c == 0xd)

// Begins with string, unchecked
#define bu(t, l) (strncmp((const char *) &s[i], t, l) == 0)

// Begins with string, checked
#define bc(t, l) (i + l < n && bu(t, l))

  while (i < n) {
    while (i < n && ws(u(0))) i++;

    if (i + 7 >= n) break;

    if (bu("//", 2)) {
      i += 2;

      while (i < n && !lt(u(0))) i++;

      if (lt(c(0))) i++;

      continue;
    }

    if (bu("/*", 2)) {
      i += 2;

      while (i + 1 < n && !bu("*/", 2)) i++;

      if (bc("/*", 2)) i += 2;

      continue;
    }

    if (bu("require", 7)) {
      i += 7;

      goto require;
    }

    else if (bu("import", 6)) {
      i += 6;

      while (i < n && ws(u(0))) i++;

      // import \*
      if (c(0) == '*') {
        i++;

        while (i < n && ws(u(0))) i++;

        // import \* as
        if (i + 3 < n && bu("as", 2) && ws(u(2))) {
          i += 3;

          while (i < n && ws(u(0))) i++;

          // import \* as [^\s]+
          while (i < n && !ws(u(0))) i++;

          while (i < n && ws(u(0))) i++;

          // import \* as [^\s]+ from
          if (bc("from", 4)) {
            i += 4;

            goto from;
          }
        }
      }

      // import {
      else if (c(0) == '{') {
        i++;

        // import {[^}]*
        while (i < n && u(0) != '}') i++;

        // import {[^}]*}
        if (c(0) == '}') {
          i++;

          while (i < n && ws(u(0))) i++;

          // import {[^}]*} from
          if (bc("from", 4)) {
            i += 4;

            goto from;
          }
        }
      }

      // import ['"]
      else if (c(0) == '\'' || c(0) == '"') {
        utf8_t e = u(0);

        j = ++i;

        while (i < n && u(0) != e) i++;

        // import ['"].*['"]
        if (c(0) == e) {
          k = i;

          i++;

          err = bare_module_lexer__add_import(env, imports, p++, &s[j], k - j, bare_module_lexer_import);
          if (err < 0) goto err;
        }
      }

      else {
        while (i < n && ws(u(0))) i++;

        // import [^\s]+
        while (i < n && !ws(u(0))) i++;

        while (i < n && ws(u(0))) i++;

        // import [^\s]+ from
        if (bc("from", 4)) {
          i += 4;

          goto from;
        }
      }
    }

    else if (bu("module", 6)) {
      i += 6;

      while (i < n && ws(u(0))) i++;

      // module\.
      if (c(0) == '.') {
        i++;

        while (i < n && ws(u(0))) i++;

        // module\.exports
        if (bc("exports", 7)) {
          i += 7;

          goto exports;
        }
      }
    }

    else if (bu("export", 6)) {
      i += 6;

      // exports
      if (c(0) == 's') {
        i++;

        goto exports;
      }

      while (i < n && ws(u(0))) i++;
    }

    else {
      i++;
    }

    continue;

  require:
    type |= bare_module_lexer_require;

    while (i < n && ws(u(0))) i++;

    // require\.
    if (c(0) == '.') {
      i++;

      while (i < n && ws(u(0))) i++;

      // require\.a
      if (i + 5 < n && u(0) == 'a') {
        i++;

        // require\.addon
        if (bu("ddon", 4)) {
          i += 4;

          while (i < n && ws(u(0))) i++;

          type |= bare_module_lexer_addon;
        }

        // require\.asset
        else if (bu("sset", 4)) {
          i += 4;

          while (i < n && ws(u(0))) i++;

          type |= bare_module_lexer_asset;
        }
      }
    }

    // require(\.(addon|asset))?\(
    if (c(0) == '(') {
      i++;

      while (i < n && ws(u(0))) i++;

      // require(\.(addon|asset))?\(['"]
      if (c(0) == '\'' || c(0) == '"') {
        utf8_t e = u(0);

        j = ++i;

        while (i < n && u(0) != e) i++;

        // require(\.(addon|asset))?\(['"].*['"]
        if (c(0) == e) {
          k = i;

          i++;

          while (i < n && ws(u(0))) i++;

          while (i < n && u(0) != ')') i++;

          // require(\.(addon|asset))?\(['"].*['"][^)]*\)
          if (c(0) == ')') {
            i++;

            err = bare_module_lexer__add_import(env, imports, p++, &s[j], k - j, type);
            if (err < 0) goto err;
          }
        }
      }
    }

    type = 0;

    continue;

  exports:
    while (i < n && ws(u(0))) i++;

    // exports =
    if (c(0) == '=') {
      i++;

      while (i < n && ws(u(0))) i++;

      // exports = \{
      if (c(0) == '{') {
      }

      // exports = require
      else if (bc("require", 7)) {
        i += 7;

        type |= bare_module_lexer_reexport;

        goto require;
      }
    }

    // exports\.
    else if (c(0) == '.') {
      i++;

      while (i < n && ws(u(0))) i++;

      j = i;

      while (i < n && !ws(u(0)) && u(0) != '=') i++;

      k = i;

      while (i < n && ws(u(0))) i++;

      // exports\.[^\s=] =
      if (c(0) == '=') {
        i++;

        err = bare_module_lexer__add_export(env, exports, q++, &s[j], k - j);
        if (err < 0) goto err;
      }
    }

    // exports\[
    else if (c(0) == '[') {
      i++;

      while (i < n && ws(u(0))) i++;

      // exports\[['"]
      if (c(0) == '\'' || c(0) == '"') {
        utf8_t e = u(0);

        j = ++i;

        while (i < n && u(0) != e) i++;

        // exports\[['"].*['"]
        if (c(0) == e) {
          k = i;

          i++;

          while (i < n && ws(u(0))) i++;

          while (i < n && u(0) != ']') i++;

          // exports\[['"].*['"][^\]]*\]
          if (c(0) == ']') {
            i++;

            while (i < n && ws(u(0))) i++;

            // exports\[['"].*['"][^\]]*\] =
            if (c(0) == '=') {
              i++;

              err = bare_module_lexer__add_export(env, exports, q++, &s[j], k - j);
              if (err < 0) goto err;
            }
          }
        }
      }
    }

    continue;

  from:
    while (i < n && ws(u(0))) i++;

    // from ['"]
    if (c(0) == '\'' || c(0) == '"') {
      utf8_t e = u(0);

      j = ++i;

      while (i < n && u(0) != e) i++;

      // from ['"].*['"]
      if (c(0) == e) {
        k = i;

        i++;

        err = bare_module_lexer__add_import(env, imports, p++, &s[j], k - j, bare_module_lexer_import);
        if (err < 0) goto err;
      }
    }

    continue;
  }

#undef u
#undef c
#undef ws
#undef lt
#undef bu
#undef bc

  return 0;

err:
  return -1;
}

#endif // BARE_MODULE_LEXER_H
