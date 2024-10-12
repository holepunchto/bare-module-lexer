#ifndef BARE_MODULE_LEXER_H
#define BARE_MODULE_LEXER_H

#include <js.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <utf.h>

typedef enum {
  bare_module_lexer_addon = 1,
  bare_module_lexer_asset = 2,
} bare_module_lexer_type;

static inline int
bare_module_lexer__add_import (js_env_t *env, js_value_t *imports, uint32_t i, const utf8_t *specifier, size_t len, bare_module_lexer_type type, bool exported) {
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
  V("exported", js_get_boolean, exported);
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

  bare_module_lexer_type type = 0;
  bool exported = false;

// Current character, unchecked
#define u(offset) (s[i + offset])

// Current character, checked
#define c(offset) (i + offset < n ? u(offset) : -1)

// Whitespace character
#define ws(c) (c == ' ' || c == '\t' || c == 0xb || c == 0xc || c == 0xa0)

// Begins with string, unchecked
#define bsu(t, l) (strncmp((const char *) &s[i], t, sizeof(t) - 1) == 0)

// Begins with string, checked
#define bsc(t, l) (i + l < n && bsu(t, l))

  while (i < n) {
    while (i < n && ws(u(0))) {
      i++;
    }

    if (bsc("require", 7)) {
      i += 7;

      goto require;
    }

    if (bsc("import", 6)) {
      i += 6;

      while (i < n && ws(u(0))) {
        i++;
      }

      // import \*
      if (c(0) == '*') {
        i++;

        while (i < n && ws(u(0))) {
          i++;
        }

        // import \* as
        if (i + 3 < n && bsu("as", 2) && ws(u(2))) {
          i += 3;

          while (i < n && ws(u(0))) {
            i++;
          }

          // import \* as [^\s]+
          while (i < n && !ws(u(0))) {
            i++;
          }

          while (i < n && ws(u(0))) {
            i++;
          }

          // import \* as [^\s]+ from
          if (bsc("from", 4)) {
            i += 4;

            goto from;
          }
        }
      }

      // import {
      else if (c(0) == '{') {
        i++;

        // import {[^}]*
        while (i < n && u(0) != '}') {
          i++;
        }

        // import {[^}]*}
        if (c(0) == '}') {
          i++;

          while (i < n && ws(u(0))) {
            i++;
          }

          // import {[^}]*} from
          if (bsc("from", 4)) {
            i += 4;

            goto from;
          }
        }
      }

      // import ['"]
      else if (c(0) == '\'' || c(0) == '"') {
        utf8_t e = u(0);

        j = ++i;

        while (i < n && u(0) != e) {
          i++;
        }

        // import ['"].*['"]
        if (c(0) == e) {
          k = i;

          i++;

          err = bare_module_lexer__add_import(env, imports, p++, &s[j], k - j, 0, false);
          if (err < 0) goto err;
        }
      }

      else {
        while (i < n && ws(u(0))) {
          i++;
        }

        // import [^\s]+
        while (i < n && !ws(u(0))) {
          i++;
        }

        while (i < n && ws(u(0))) {
          i++;
        }

        // import [^\s]+ from
        if (bsc("from", 4)) {
          i += 4;

          goto from;
        }
      }
    }

    if (bsc("module", 6)) {
      i += 6;

      while (i < n && ws(u(0))) {
        i++;
      }

      // module\.
      if (c(0) == '.') {
        i++;

        while (i < n && ws(u(0))) {
          i++;
        }

        // module\.exports
        if (bsc("exports", 7)) {
          i += 7;

          goto exports;
        }
      }
    }

    if (bsc("export", 6)) {
      // exports
      if (c(6) == 's') {
        i += 7;

        goto exports;
      }

      i += 6;

      while (i < n && ws(u(0))) {
        i++;
      }
    }

    i++;

    continue;

  require:
    while (i < n && ws(u(0))) {
      i++;
    }

    // require\.
    if (c(0) == '.') {
      i++;

      while (i < n && ws(u(0))) {
        i++;
      }

      // require\.a
      if (i + 5 < n && u(0) == 'a') {
        i++;

        // require\.addon
        if (bsu("ddon", 4)) {
          i += 4;

          while (i < n && ws(u(0))) {
            i++;
          }

          type = bare_module_lexer_addon;
        }

        // require\.asset
        else if (bsu("sset", 4)) {
          i += 4;

          while (i < n && ws(u(0))) {
            i++;
          }

          type = bare_module_lexer_asset;
        }
      }
    }

    // require(\.(addon|asset))?\(
    if (c(0) == '(') {
      i++;

      while (i < n && ws(u(0))) {
        i++;
      }

      // require(\.(addon|asset))?\(['"]
      if (c(0) == '\'' || c(0) == '"') {
        utf8_t e = u(0);

        j = ++i;

        while (i < n && u(0) != e) {
          i++;
        }

        // require(\.(addon|asset))?\(['"].*['"]
        if (c(0) == e) {
          k = i;

          i++;

          while (i < n && ws(u(0))) {
            i++;
          }

          while (i < n && u(0) != ')') {
            i++;
          }

          // require(\.(addon|asset))?\(['"].*['"][^)]*\)
          if (c(0) == ')') {
            i++;

            err = bare_module_lexer__add_import(env, imports, p++, &s[j], k - j, type, exported);
            if (err < 0) goto err;
          }
        }
      }
    }

    type = 0;
    exported = false;

    continue;

  exports:
    while (i < n && ws(u(0))) {
      i++;
    }

    // exports =
    if (c(0) == '=') {
      i++;

      while (i < n && ws(u(0))) {
        i++;
      }

      // exports = \{
      if (c(0) == '{') {
      }

      // exports = require
      else if (bsc("require", 7)) {
        i += 7;

        exported = true;

        goto require;
      }
    }

    // exports\.
    else if (c(0) == '.') {
      i++;

      while (i < n && ws(u(0))) {
        i++;
      }

      j = i;

      while (i < n && !ws(u(0)) && u(0) != '=') {
        i++;
      }

      k = i;

      while (i < n && ws(u(0))) {
        i++;
      }

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

      while (i < n && ws(u(0))) {
        i++;
      }

      // exports\[['"]
      if (c(0) == '\'' || c(0) == '"') {
        utf8_t e = u(0);

        j = ++i;

        while (i < n && u(0) != e) {
          i++;
        }

        // exports\[['"].*['"]
        if (c(0) == e) {
          k = i;

          i++;

          while (i < n && ws(u(0))) {
            i++;
          }

          while (i < n && u(0) != ']') {
            i++;
          }

          // exports\[['"].*['"][^\]]*\]
          if (c(0) == ']') {
            i++;

            while (i < n && ws(u(0))) {
              i++;
            }

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
    while (i < n && ws(u(0))) {
      i++;
    }

    // from ['"]
    if (c(0) == '\'' || c(0) == '"') {
      utf8_t e = u(0);

      j = ++i;

      while (i < n && u(0) != e) {
        i++;
      }

      // from ['"].*['"]
      if (c(0) == e) {
        k = i;

        i++;

        err = bare_module_lexer__add_import(env, imports, p++, &s[j], k - j, 0, exported);
        if (err < 0) goto err;
      }
    }

    continue;
  }

#undef u
#undef c
#undef ws
#undef bsu
#undef bsc

  return 0;

err:
  return -1;
}

#endif // BARE_MODULE_LEXER_H
