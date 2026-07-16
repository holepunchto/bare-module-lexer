#ifndef BARE_MODULE_LEXER_H
#define BARE_MODULE_LEXER_H

#include <assert.h>
#include <js.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <utf.h>

enum {
  bare_module_lexer_require = 0x1,
  bare_module_lexer_import = 0x2,
  bare_module_lexer_dynamic = 0x4,
  bare_module_lexer_addon = 0x8,
  bare_module_lexer_asset = 0x10,
  bare_module_lexer_resolve = 0x20,
  bare_module_lexer_reexport = 0x40,
};

// Previous significant token classification - just enough context to (a)
// reject keyword matches that are actually member accesses and (b) decide
// whether '/' starts a regular expression or is a division operator.
enum {
  bare_module_lexer__prev_op = 0,   // After an operator or statement boundary - an expression follows
  bare_module_lexer__prev_expr = 1, // After an identifier, literal, ')', or ']' - an expression just ended
  bare_module_lexer__prev_dot = 2   // After '.' - a property name follows
};

// Maximum number of nested template substitutions tracked. Deeper nesting
// degrades to plain brace counting rather than failing - the lexer must
// never throw.
#define BARE_MODULE_LEXER__TEMPLATE_DEPTH 64

static inline bool
bare_module_lexer__is_line_terminator(uint8_t c) {
  return c == 0xa || c == 0xd;
}

static inline bool
bare_module_lexer__is_whitespace(uint8_t c) {
  return c == ' ' || c == '\t' || c == 0xb || c == 0xc || bare_module_lexer__is_line_terminator(c);
}

static inline bool
bare_module_lexer__is_id_start(uint8_t c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || c == '$';
}

static inline bool
bare_module_lexer__is_id(uint8_t c) {
  return bare_module_lexer__is_id_start(c) || (c >= '0' && c <= '9');
}

// True when the byte can continue an identifier, including any non-ASCII
// byte: Unicode identifiers are not validated, only treated as opaque runs
// so keyword matches can't begin inside them.
static inline bool
bare_module_lexer__is_id_like(uint8_t c) {
  return bare_module_lexer__is_id(c) || c >= 0x80;
}

// True when the byte can start an identifier, with the same opaque treatment
// of non-ASCII bytes.
static inline bool
bare_module_lexer__is_id_like_start(uint8_t c) {
  return bare_module_lexer__is_id_start(c) || c >= 0x80;
}

static inline int
bare_module_lexer__add_position(js_env_t *env, js_value_t *entry, size_t statement_start, size_t input_start, size_t input_end) {
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
    assert(err == 0); \
  }

  V(0, statement_start);
  V(1, input_start);
  V(2, input_end);
#undef V

  err = js_set_named_property(env, entry, "position", position);
  assert(err == 0);

  return 0;
}

static inline int
bare_module_lexer__add_import(js_env_t *env, js_value_t *imports, uint32_t *i, const utf8_t *source, size_t import_start, size_t specifier_start, size_t specifier_end, int type, js_value_t *names, js_value_t *attributes) {
  assert(specifier_end >= specifier_start);

  int err;

  js_value_t *entry;
  err = js_create_object(env, &entry);
  assert(err == 0);

  err = js_set_element(env, imports, *i, entry);
  assert(err == 0);

#define V(key, fn, ...) \
  { \
    js_value_t *val; \
    err = fn(env, __VA_ARGS__, &val); \
    assert(err == 0); \
    err = js_set_named_property(env, entry, key, val); \
    assert(err == 0); \
  }

  V("specifier", js_create_string_utf8, &source[specifier_start], specifier_end - specifier_start);
  V("type", js_create_uint32, type);
#undef V

  if (names == NULL) {
    err = js_create_array(env, &names);
    assert(err == 0);
  }

  err = js_set_named_property(env, entry, "names", names);
  assert(err == 0);

  if (attributes == NULL) {
    err = js_create_object(env, &attributes);
    assert(err == 0);
  }

  err = js_set_named_property(env, entry, "attributes", attributes);
  assert(err == 0);

  err = bare_module_lexer__add_position(env, entry, import_start, specifier_start, specifier_end);
  assert(err == 0);

  *i += 1;

  return 0;
}

static inline int
bare_module_lexer__add_export(js_env_t *env, js_value_t *exports, uint32_t *i, const utf8_t *source, size_t export_start, size_t name_start, size_t name_end) {
  assert(name_end >= name_start);

  int err;

  js_value_t *entry;
  err = js_create_object(env, &entry);
  assert(err == 0);

  err = js_set_element(env, exports, *i, entry);
  assert(err == 0);

#define V(key, fn, ...) \
  { \
    js_value_t *val; \
    err = fn(env, __VA_ARGS__, &val); \
    assert(err == 0); \
    err = js_set_named_property(env, entry, key, val); \
    assert(err == 0); \
  }

  V("name", js_create_string_utf8, &source[name_start], name_end - name_start);
#undef V

  err = bare_module_lexer__add_position(env, entry, export_start, name_start, name_end);
  assert(err == 0);

  *i += 1;

  return 0;
}

static inline int
bare_module_lexer__add_name(js_env_t *env, js_value_t **names, uint32_t *i, const utf8_t *name, size_t name_len) {
  int err;

  if (*names == NULL) {
    err = js_create_array(env, names);
    assert(err == 0);

    *i = 0;
  }

  js_value_t *string;
  err = js_create_string_utf8(env, name, name_len, &string);
  assert(err == 0);

  err = js_set_element(env, *names, *i, string);
  assert(err == 0);

  *i += 1;

  return 0;
}

static inline int
bare_module_lexer__add_attribute(js_env_t *env, js_value_t **attributes, const utf8_t *key, size_t key_len, const utf8_t *value, size_t value_len) {
  int err;

  if (*attributes == NULL) {
    err = js_create_object(env, attributes);
    assert(err == 0);
  }

  js_value_t *property;
  err = js_create_property_key_utf8(env, key, key_len, &property);
  assert(err == 0);

  js_value_t *string;
  err = js_create_string_utf8(env, value, value_len, &string);
  assert(err == 0);

  err = js_set_property(env, *attributes, property, string);
  assert(err == 0);

  return 0;
}

// Current character, unchecked
#define u(offset) ((uint8_t) s[i + offset])

// Current character, checked
#define c(offset) (i + offset < n ? u(offset) : -1)

#define lt   bare_module_lexer__is_line_terminator
#define ws   bare_module_lexer__is_whitespace
#define ids  bare_module_lexer__is_id_start
#define id   bare_module_lexer__is_id
#define idl  bare_module_lexer__is_id_like
#define idsl bare_module_lexer__is_id_like_start

// True iff the bytes at i spell the keyword `kw` of length `kwl` with a word
// boundary after it (end of input or a non-identifier byte).
static inline bool
bare_module_lexer__at_kw(const utf8_t *s, size_t n, size_t i, const char *kw, size_t kwl) {
  return i + kwl <= n && memcmp(&s[i], kw, kwl) == 0 && (i + kwl == n || !idl(s[i + kwl]));
}

// True iff the identifier span s[ks..ks+kl] equals the keyword `kw` of
// length `kwl`.
static inline bool
bare_module_lexer__match_kw(const utf8_t *s, size_t ks, size_t kl, const char *kw, size_t kwl) {
  return kl == kwl && memcmp(&s[ks], kw, kwl) == 0;
}

// Skip whitespace and comments.
static inline size_t
bare_module_lexer__skip_trivia(const utf8_t *s, size_t n, size_t i) {
  while (i < n) {
    if (ws(u(0))) {
      i++;
    } else if (i + 1 < n && u(0) == '/' && u(1) == '/') {
      i += 2;
      while (i < n && !lt(u(0))) i++;
    } else if (i + 1 < n && u(0) == '/' && u(1) == '*') {
      i += 2;
      while (i + 1 < n && !(u(0) == '*' && u(1) == '/')) i++;
      if (i + 1 < n) i += 2;
      else i = n;
    } else {
      break;
    }
  }

  return i;
}

// Lex a string literal at *result, which must point at the opening quote.
// Fills *start/*end with the content bounds and leaves the cursor past the
// closing quote. Returns false when the string is unterminated, with the
// cursor at end of input.
static inline bool
bare_module_lexer__lex_string(const utf8_t *s, size_t n, size_t *result, size_t *start, size_t *end) {
  size_t i = *result;
  uint8_t e = s[i];

  i++;
  *start = i;

  while (i < n) {
    if (s[i] == '\\') {
      i += 2;
      continue;
    }

    if (s[i] == e) {
      *end = i;
      *result = i + 1;
      return true;
    }

    i++;
  }

  *start = *end = n;
  *result = n;
  return false;
}

// Scan template literal content from just past the opening '`' (or just
// past the '}' that closed a substitution). Stops after the terminating '`'
// and returns false, or after a '${' and returns true so the caller can lex
// the substitution as ordinary code.
static inline bool
bare_module_lexer__lex_template(const utf8_t *s, size_t n, size_t *result) {
  size_t i = *result;

  while (i < n) {
    if (u(0) == '\\') {
      i += 2;
      continue;
    }

    if (u(0) == '`') {
      i++;
      break;
    }

    if (u(0) == '$' && i + 1 < n && u(1) == '{') {
      *result = i + 2;
      return true;
    }

    i++;
  }

  *result = i > n ? n : i;
  return false;
}

// Skip a template literal wholesale. Assumes s[i] == '`'. Substitutions are
// tracked by brace counting only, which is good enough for a skip.
static inline size_t
bare_module_lexer__skip_template(const utf8_t *s, size_t n, size_t i) {
  int braces = 0;
  bool subst = false;

  i++;

  while (i < n) {
    uint8_t ch = u(0);

    if (ch == '\\') {
      i += 2;
      continue;
    }

    if (!subst) {
      if (ch == '`') return i + 1;

      if (ch == '$' && i + 1 < n && u(1) == '{') {
        subst = true;
        braces = 0;
        i += 2;
        continue;
      }

      i++;
      continue;
    }

    if (ch == '\'' || ch == '"') {
      size_t vs, ve;
      bare_module_lexer__lex_string(s, n, &i, &vs, &ve);
      continue;
    }

    if (ch == '{') braces++;
    else if (ch == '}') {
      if (braces == 0) subst = false;
      else braces--;
    }

    i++;
  }

  return i;
}

// Skip a property value expression inside an object literal: Everything up
// to the next ',' or closing bracket at the current depth. Strings,
// templates, comments, and nested brackets are skipped wholesale.
static inline size_t
bare_module_lexer__skip_value(const utf8_t *s, size_t n, size_t i) {
  int depth = 0;

  while (i < n) {
    uint8_t ch = u(0);

    if (ws(ch) || (ch == '/' && i + 1 < n && (u(1) == '/' || u(1) == '*'))) {
      size_t j = bare_module_lexer__skip_trivia(s, n, i);
      if (j == i) i++;
      else i = j;
      continue;
    }

    if (ch == '\'' || ch == '"') {
      size_t vs, ve;
      bare_module_lexer__lex_string(s, n, &i, &vs, &ve);
      continue;
    }

    if (ch == '`') {
      i = bare_module_lexer__skip_template(s, n, i);
      continue;
    }

    if (ch == '(' || ch == '[' || ch == '{') {
      depth++;
      i++;
      continue;
    }

    if (ch == ')' || ch == ']' || ch == '}') {
      if (depth == 0) return i;
      depth--;
      i++;
      continue;
    }

    if (ch == ',' && depth == 0) return i;

    i++;
  }

  return i;
}

// True when any byte in [a, b) is a line terminator.
static inline bool
bare_module_lexer__has_line_terminator(const utf8_t *s, size_t a, size_t b) {
  for (size_t k = a; k < b; k++) {
    if (lt(s[k])) return true;
  }

  return false;
}

// Heuristic ASI for type expressions: a line terminator at the outer nesting
// level continues the type only when the previous significant byte is a
// trailing operator or the next one is a leading operator. Being wrong here
// only ever stops a type early - the remainder is then lexed as ordinary
// code - so it never swallows a following statement's require()/import().
static inline bool
bare_module_lexer__type_continues(uint8_t last, uint8_t next) {
  switch (last) {
  case '=':
  case '|':
  case '&':
  case ',':
  case '<':
  case '(':
  case '[':
  case '{':
  case '.':
  case '?':
  case ':':
    return true;
  }

  switch (next) {
  case '|':
  case '&':
  case '.':
  case '?':
  case ':':
    return true;
  }

  return false;
}

// Skip a TypeScript type expression starting at i - a type alias right-hand
// side or a ':' annotation. Types are erased at compile time, so any
// import(...) inside one is not a runtime dependency and must not be lexed as
// a module import. Strings, templates, and comments are skipped wholesale;
// '()[]{}' and '<>' nesting is tracked so commas and arrows inside generics
// and function types don't end the scan early. Stops at the outer level on
// ';', ',', or '=' (but not '=>'), on a closing bracket that would underflow
// an enclosing list, on a non-continuing line terminator, or at end of input.
static inline size_t
bare_module_lexer__skip_type(const utf8_t *s, size_t n, size_t i) {
  int depth = 0; // () [] {} nesting
  int angle = 0; // <> nesting

  uint8_t last = 0; // Previous significant byte, for ASI continuation

  while (i < n) {
    uint8_t ch = u(0);

    // Line terminator - at the outer level it may end the type.
    if (lt(ch)) {
      if (depth == 0 && angle == 0) {
        size_t p = bare_module_lexer__skip_trivia(s, n, i);

        if (!bare_module_lexer__type_continues(last, p < n ? s[p] : 0)) break;

        i = p;
        continue;
      }

      i++;
      continue;
    }

    // Other whitespace and comments.
    if (ws(ch) || (ch == '/' && i + 1 < n && (u(1) == '/' || u(1) == '*'))) {
      size_t j = bare_module_lexer__skip_trivia(s, n, i);

      if (j == i) i++;
      else i = j;

      continue;
    }

    // Strings and templates are opaque.
    if (ch == '\'' || ch == '"') {
      size_t vs, ve;
      bare_module_lexer__lex_string(s, n, &i, &vs, &ve);
      last = '"';
      continue;
    }

    if (ch == '`') {
      i = bare_module_lexer__skip_template(s, n, i);
      last = '`';
      continue;
    }

    if (depth == 0 && angle == 0) {
      // A ';', ',', or bare '=' at the outer level ends the type.
      if (ch == ';' || ch == ',') break;

      if (ch == '=' && !(i + 1 < n && u(1) == '>')) break;

      // A closing bracket at the outer level belongs to an enclosing list.
      if (ch == ')' || ch == ']' || ch == '}') break;
    }

    // '=>' arrow - consume both bytes so the '>' isn't taken as a '<>' close.
    if (ch == '=' && i + 1 < n && u(1) == '>') {
      last = '>';
      i += 2;
      continue;
    }

    if (ch == '(' || ch == '[' || ch == '{') depth++;
    else if (ch == ')' || ch == ']' || ch == '}') depth--;
    else if (ch == '<') angle++;
    else if (ch == '>' && angle > 0) angle--;

    last = ch;
    i++;
  }

  return i;
}

// Maximum destructuring pattern nesting tracked. Deeper nesting has its
// entries skipped rather than failing - the lexer must never throw.
#define BARE_MODULE_LEXER__PATTERN_DEPTH 32

// Lex a destructuring pattern at *result (which must point at '{' or '['),
// emitting an export for each binding name. Best effort: String and
// computed keys and patterns nested deeper than the cap have their entries
// skipped wholesale.
static inline int
bare_module_lexer__lex_pattern(js_env_t *env, js_value_t *exports, uint32_t *el, const utf8_t *s, size_t n, size_t es, size_t *result, int depth) {
  int err;

  size_t i = *result;

  bool object = s[i] == '{';
  uint8_t close = object ? '}' : ']';

  i++;

  while (i < n) {
    i = bare_module_lexer__skip_trivia(s, n, i);
    if (i >= n) break;

    if (u(0) == close) {
      i++;
      break;
    }

    if (u(0) == ',') {
      i++;
      continue;
    }

    // Rest element '...name'.
    if (i + 2 < n && u(0) == '.' && u(1) == '.' && u(2) == '.') {
      i = bare_module_lexer__skip_trivia(s, n, i + 3);

      if (i < n && idsl(u(0))) {
        size_t ns = i++;

        while (i < n && idl(u(0))) i++;

        err = bare_module_lexer__add_export(env, exports, el, s, es, ns, i);
        if (err < 0) return err;
      }

      continue;
    }

    // Binding name or 'key: target'.
    if (idsl(u(0))) {
      size_t ns = i++;

      while (i < n && idl(u(0))) i++;

      size_t ne = i;

      i = bare_module_lexer__skip_trivia(s, n, i);

      // 'key: target' - the target holds the binding.
      if (object && i < n && u(0) == ':') {
        i = bare_module_lexer__skip_trivia(s, n, i + 1);

        if (i < n && (u(0) == '{' || u(0) == '[')) {
          if (depth < BARE_MODULE_LEXER__PATTERN_DEPTH) {
            err = bare_module_lexer__lex_pattern(env, exports, el, s, n, es, &i, depth + 1);
            if (err < 0) return err;
          } else {
            i = bare_module_lexer__skip_value(s, n, i);
          }
        } else if (i < n && idsl(u(0))) {
          ns = i++;

          while (i < n && idl(u(0))) i++;

          ne = i;

          err = bare_module_lexer__add_export(env, exports, el, s, es, ns, ne);
          if (err < 0) return err;
        }
      }

      // Shorthand binding.
      else {
        err = bare_module_lexer__add_export(env, exports, el, s, es, ns, ne);
        if (err < 0) return err;
      }

      i = bare_module_lexer__skip_trivia(s, n, i);

      // Default value - skip to the next entry.
      if (i < n && u(0) == '=') {
        i = bare_module_lexer__skip_value(s, n, i + 1);
      }

      continue;
    }

    // Nested pattern as an array element.
    if (!object && (u(0) == '{' || u(0) == '[')) {
      if (depth < BARE_MODULE_LEXER__PATTERN_DEPTH) {
        err = bare_module_lexer__lex_pattern(env, exports, el, s, n, es, &i, depth + 1);
        if (err < 0) return err;
      } else {
        i = bare_module_lexer__skip_value(s, n, i);
      }

      i = bare_module_lexer__skip_trivia(s, n, i);

      if (i < n && u(0) == '=') {
        i = bare_module_lexer__skip_value(s, n, i + 1);
      }

      continue;
    }

    // String or computed key, holes, anything else - skip the entry.
    {
      size_t j = bare_module_lexer__skip_value(s, n, i);

      if (j == i) i++;
      else i = j;
    }
  }

  *result = i;

  return 0;
}

// Keywords after which a '/' starts a regular expression rather than a
// division - they end a statement or expect an expression to follow.
typedef struct {
  const char *kw;
  uint8_t len;
} bare_module_lexer__kw_t;

static const bare_module_lexer__kw_t bare_module_lexer__op_kws[] = {
  {"in", 2},
  {"of", 2},
  {"do", 2},
  {"new", 3},
  {"else", 4},
  {"case", 4},
  {"void", 4},
  {"yield", 5},
  {"await", 5},
  {"throw", 5},
  {"delete", 6},
  {"return", 6},
  {"typeof", 6},
  {"instanceof", 10},
};

// Classify an identifier span: Keywords that expect an expression to follow
// leave `prev` at op so a following '/' lexes as a regular expression;
// everything else ends an expression.
static inline int
bare_module_lexer__classify_kw(const utf8_t *s, size_t ks, size_t kl) {
  if (kl >= 2 && kl <= 10) {
    uint8_t c0 = s[ks];

    for (size_t k = 0; k < sizeof(bare_module_lexer__op_kws) / sizeof(bare_module_lexer__op_kws[0]); k++) {
      const bare_module_lexer__kw_t *kw = &bare_module_lexer__op_kws[k];

      if (kw->len == kl && (uint8_t) kw->kw[0] == c0 && memcmp(&s[ks], kw->kw, kl) == 0) {
        return bare_module_lexer__prev_op;
      }
    }
  }

  return bare_module_lexer__prev_expr;
}

static inline int
bare_module_lexer__lex_regex(const utf8_t *s, size_t n, size_t i, size_t *result) {
  i++;

  bool in_character_class = false;

  while (i < n) {
    if (in_character_class) {
      if (u(0) == ']') {
        in_character_class = false;
      } else if (c(0) == '\\') {
        i++;
      }
    } else if (u(0) == '/') {
      i++;

      while (i < n && u(0) >= 'a' && u(0) <= 'z') i++;

      *result = i;

      return 0;
    } else if (u(0) == '[') {
      in_character_class = true;
    } else if (u(0) == '\\') {
      i++;
    } else if (u(0) == ';' || lt(u(0))) {
      return -1;
    }

    i++;
  }

  return -1;
}

// Parse a '.addon[.resolve]', '.asset', or '.resolve' member suffix at i
// (s[i] == '.'), OR'ing the matching type flags. Returns the index past the
// suffix and any trailing trivia, or i unchanged when the members don't
// match - the caller then lexes the '.' as ordinary member access.
static inline size_t
bare_module_lexer__lex_member_suffix(const utf8_t *s, size_t n, size_t i, int *type) {
  size_t p = bare_module_lexer__skip_trivia(s, n, i + 1);

  if (bare_module_lexer__at_kw(s, n, p, "addon", 5)) {
    *type |= bare_module_lexer_addon;

    p = bare_module_lexer__skip_trivia(s, n, p + 5);

    if (p < n && s[p] == '.') {
      size_t q = bare_module_lexer__skip_trivia(s, n, p + 1);

      if (!bare_module_lexer__at_kw(s, n, q, "resolve", 7)) return i;

      *type |= bare_module_lexer_resolve;

      p = bare_module_lexer__skip_trivia(s, n, q + 7);
    }

    return p;
  }

  if (bare_module_lexer__at_kw(s, n, p, "asset", 5)) {
    *type |= bare_module_lexer_asset;

    return bare_module_lexer__skip_trivia(s, n, p + 5);
  }

  if (bare_module_lexer__at_kw(s, n, p, "resolve", 7)) {
    *type |= bare_module_lexer_resolve;

    return bare_module_lexer__skip_trivia(s, n, p + 7);
  }

  return i;
}

// Detect a TypeScript inline `type` modifier at the start of an
// import/export specifier. The modifier is the keyword `type` followed by
// another binding name - an identifier or string literal - rather than by
// `as`, `,`, or the closing brace. In `{ type }`, `{ type, x }`, and
// `{ type as x }` the word `type` is itself the imported/exported name, not a
// modifier. On a match, returns the index of the binding name that follows so
// the specifier can be skipped; otherwise returns i unchanged.
static inline size_t
bare_module_lexer__lex_type_modifier(const utf8_t *s, size_t n, size_t i) {
  if (!bare_module_lexer__at_kw(s, n, i, "type", 4)) return i;

  size_t p = bare_module_lexer__skip_trivia(s, n, i + 4);

  if (p >= n) return i;

  if (bare_module_lexer__at_kw(s, n, p, "as", 2)) return i;

  if (idsl(s[p]) || s[p] == '\'' || s[p] == '"') return p;

  return i;
}

static inline int
bare_module_lexer__lex_import_attributes(js_env_t *env, js_value_t **attributes, const utf8_t *s, size_t n, size_t i, size_t *result) {
  int err;

  size_t ks; // Key start
  size_t ke; // Key end

  size_t vs; // Value start
  size_t ve; // Value end

  i = bare_module_lexer__skip_trivia(s, n, i);

  if (c(0) == '{') {
    i++;

    while (c(0) != '}') {
      i = bare_module_lexer__skip_trivia(s, n, i);

      if (i < n && idsl(u(0))) {
        ks = i++;

        while (i < n && idl(u(0))) i++;

        ke = i;
      } else if (c(0) == '\'' || c(0) == '"') {
        if (!bare_module_lexer__lex_string(s, n, &i, &ks, &ke)) break;
      } else break;

      i = bare_module_lexer__skip_trivia(s, n, i);

      if (c(0) != ':') break;

      i++;

      i = bare_module_lexer__skip_trivia(s, n, i);

      if (c(0) != '\'' && c(0) != '"') break;

      if (!bare_module_lexer__lex_string(s, n, &i, &vs, &ve)) break;

      err = bare_module_lexer__add_attribute(env, attributes, &s[ks], ke - ks, &s[vs], ve - vs);
      if (err < 0) return err;

      i = bare_module_lexer__skip_trivia(s, n, i);

      if (c(0) != ',') break;

      i++;
    }
  }

  *result = i;

  return 0;
}

// Lex the '{ ... }' of an import or re-export specifier list, emitting a name
// for each value binding. TypeScript's inline `type` modifier erases the
// binding it precedes. When the list is non-empty and every binding is
// type-only, *only_type is set so the caller can erase the whole declaration;
// it is left false for an empty list, which is a bindingless side-effect
// import. *only_type may be NULL when the caller has a value binding of its
// own (a default or namespace import) and so never erases.
static inline int
bare_module_lexer__lex_import_names(js_env_t *env, js_value_t **names, uint32_t *nl, const utf8_t *s, size_t n, size_t i, size_t *result, bool *only_type) {
  int err;

  size_t ns; // Name start
  size_t ne; // Name end

  bool any = false;   // Saw at least one binding
  bool value = false; // Saw at least one value (non-type) binding

  i = bare_module_lexer__skip_trivia(s, n, i);

  if (c(0) == '{') {
    i++;

    while (c(0) != '}') {
      i = bare_module_lexer__skip_trivia(s, n, i);

      // A leading `type` modifier erases the binding it precedes.
      size_t p = bare_module_lexer__lex_type_modifier(s, n, i);
      bool is_type = p != i;

      if (is_type) i = p;

      // Name is an identifier or a string literal (arbitrary module
      // namespace names, e.g. 'import { "x" as y }').
      if (i < n && idsl(u(0))) {
        ns = i++;

        while (i < n && idl(u(0))) i++;

        ne = i;
      } else if (c(0) == '\'' || c(0) == '"') {
        if (!bare_module_lexer__lex_string(s, n, &i, &ns, &ne)) break;
      } else break;

      any = true;

      // A type-only binding is erased - keep no name for it.
      if (!is_type) {
        value = true;

        err = bare_module_lexer__add_name(env, names, nl, &s[ns], ne - ns);
        assert(err == 0);
      }

      i = bare_module_lexer__skip_trivia(s, n, i);

      // [name] as
      if (bare_module_lexer__at_kw(s, n, i, "as", 2)) {
        i = bare_module_lexer__skip_trivia(s, n, i + 2);

        // [name] as [alias] - the alias is an identifier or string literal.
        if (c(0) == '\'' || c(0) == '"') {
          size_t als, ale;

          if (!bare_module_lexer__lex_string(s, n, &i, &als, &ale)) break;
        } else {
          while (i < n && idl(u(0))) i++;
        }

        i = bare_module_lexer__skip_trivia(s, n, i);
      }

      if (c(0) != ',') break;

      i++;
    }

    while (i < n && u(0) != '}') i++;

    if (c(0) == '}') i++;
  }

  if (only_type != NULL) *only_type = any && !value;

  *result = i;

  return 0;
}

// Match the base URL argument of an artifact or resolve call. The specifier
// can only be resolved statically when the base is the referring module
// itself: '__filename' in a CommonJS module, or 'import.meta.url' /
// 'import.meta.filename' in an ES module. Returns the position past the base
// when matched, or i unchanged when it is any other value.
static inline size_t
bare_module_lexer__lex_base(const utf8_t *s, size_t n, size_t i, int type) {
  if ((type & bare_module_lexer_require) && bare_module_lexer__at_kw(s, n, i, "__filename", 10)) {
    return i + 10;
  }

  if ((type & bare_module_lexer_import) && bare_module_lexer__at_kw(s, n, i, "import", 6)) {
    size_t p = bare_module_lexer__skip_trivia(s, n, i + 6);

    if (p < n && s[p] == '.') {
      p = bare_module_lexer__skip_trivia(s, n, p + 1);

      if (bare_module_lexer__at_kw(s, n, p, "meta", 4)) {
        p = bare_module_lexer__skip_trivia(s, n, p + 4);

        if (p < n && s[p] == '.') {
          p = bare_module_lexer__skip_trivia(s, n, p + 1);

          if (bare_module_lexer__at_kw(s, n, p, "url", 3)) return p + 3;
          if (bare_module_lexer__at_kw(s, n, p, "filename", 8)) return p + 8;
        }
      }
    }
  }

  return i;
}

// Parse a call tail '([\s]*<string>[, { with: { ... } }][^)]*)' at *result,
// which must point at the '('. On a match, fills the specifier bounds and
// any import attributes and leaves the cursor past the ')'. When the
// argument list doesn't start with a string literal, the cursor is left
// just inside the '(' so the arguments lex as ordinary code.
//
// For the artifact and resolve variants ('require.asset', 'require.resolve',
// 'require.addon[.resolve]', and the 'import.meta' equivalents) the second
// argument is a base URL rather than an options object, and the call matches
// only when that base is the referring module itself.
static inline int
bare_module_lexer__lex_call(js_env_t *env, js_value_t **attributes, const utf8_t *s, size_t n, size_t *result, size_t *ss, size_t *se, int type, bool *matched) {
  int err;

  size_t i = *result;

  *matched = false;

  i++;

  i = bare_module_lexer__skip_trivia(s, n, i);

  if (c(0) == '\'' || c(0) == '"') {
    if (bare_module_lexer__lex_string(s, n, &i, ss, se)) {
      i = bare_module_lexer__skip_trivia(s, n, i);

      // The string is the specifier only when it is the complete first
      // argument, i.e. immediately followed by ')' or ','. Anything else, such
      // as '+', means the string is the start of a larger expression and is not
      // a specifier, so the call is left unmatched.
      if (c(0) == ')') {
        i++;

        *matched = true;
      } else if (c(0) == ',') {
        i++;

        i = bare_module_lexer__skip_trivia(s, n, i);

        // The artifact and resolve variants take a base URL as their second
        // argument. It matches only when it is the referring module itself;
        // any other base is a runtime value, so the call is left unmatched
        // and the argument lexes as ordinary code.
        if (type & (bare_module_lexer_addon | bare_module_lexer_asset | bare_module_lexer_resolve)) {
          size_t p = bare_module_lexer__lex_base(s, n, i, type);

          if (p != i) {
            i = bare_module_lexer__skip_trivia(s, n, p);

            if (c(0) == ')') {
              i++;

              *matched = true;
            }
          }
        } else {
          // An options object '{ with: { ... } }'.
          if (c(0) == '{') {
            size_t p = bare_module_lexer__skip_trivia(s, n, i + 1);

            if (bare_module_lexer__at_kw(s, n, p, "with", 4)) {
              p = bare_module_lexer__skip_trivia(s, n, p + 4);

              if (p < n && s[p] == ':') {
                err = bare_module_lexer__lex_import_attributes(env, attributes, s, n, p + 1, &i);
                if (err < 0) return err;
              }
            }
          }

          while (i < n && u(0) != ')') i++;

          if (c(0) == ')') {
            i++;

            *matched = true;
          }
        }
      }
    }
  }

  *result = i;

  return 0;
}

static inline int
bare_module_lexer__lex(js_env_t *env, js_value_t *imports, js_value_t *exports, const utf8_t *s, size_t n) {
  int err;

  size_t i = 0;

  size_t is;       // Import start
  uint32_t il = 0; // Import count

  size_t es;       // Export start
  uint32_t el = 0; // Export count

  size_t ss; // Source start
  size_t se; // Source end

  int type;

  js_value_t *names;
  uint32_t nl; // Names count

  js_value_t *attributes;

  bool matched;

  // Set when an import's binding list is entirely TypeScript type-only
  // specifiers, so the declaration is erased rather than emitted.
  bool type_only;

  // Previous significant token classification - see enum at top.
  int prev = bare_module_lexer__prev_op;

  // Open template substitutions: The brace depth at which each '${' was
  // entered, so the matching '}' can resume template content scanning.
  size_t template_stack[BARE_MODULE_LEXER__TEMPLATE_DEPTH];
  int template_depth = 0;
  int brace_depth = 0;
  int paren_depth = 0;
  int bracket_depth = 0;

  // An exported declarator list ('export const a = 1, b = 2') whose
  // initializer is being lexed as ordinary code. The list resumes at the
  // next ',' at the recorded depths, so initializers may contain require()
  // and import() and still be seen.
  bool declarators = false;
  int d_brace = 0;
  int d_paren = 0;
  int d_bracket = 0;

  // A 'module.exports = { ... }' object literal whose property values are
  // being lexed as ordinary code. The property list resumes at the next ','
  // at the recorded depths, so values may contain require() and import()
  // and still be seen.
  bool properties = false;
  int p_brace = 0;
  int p_paren = 0;
  int p_bracket = 0;

  while (i < n) {
    size_t j = bare_module_lexer__skip_trivia(s, n, i);

    // A line terminator after a complete initializer ends a pending
    // declarator list (the lexical approximation of ASI) - unless the next
    // token is the ',' continuing it (comma-first style).
    if (declarators && prev == bare_module_lexer__prev_expr && brace_depth == d_brace && paren_depth == d_paren && bracket_depth == d_bracket) {
      while (i < j && !lt(u(0))) i++;

      if (i < j && (j >= n || s[j] != ',')) declarators = false;
    }

    i = j;

    if (i >= n) break;

    uint8_t ch = u(0);

    // String literal.
    if (ch == '\'' || ch == '"') {
      bare_module_lexer__lex_string(s, n, &i, &ss, &se);
      prev = bare_module_lexer__prev_expr;
      continue;
    }

    // Template literal. Content is inert, but each '${' substitution hands
    // control back to this loop so the code inside is lexed normally; the
    // matching '}' resumes content scanning via the substitution stack.
    if (ch == '`') {
      i++;

      template : if (bare_module_lexer__lex_template(s, n, &i)) {
        if (template_depth < BARE_MODULE_LEXER__TEMPLATE_DEPTH) {
          template_stack[template_depth++] = brace_depth;
          prev = bare_module_lexer__prev_op;
          continue;
        }

        // Nested too deeply to track - skip the rest of the template
        // wholesale rather than failing.
        i = bare_module_lexer__skip_template(s, n, i - 1);
      }

      prev = bare_module_lexer__prev_expr;
      continue;
    }

    // Regular expression or division.
    if (ch == '/') {
      if (prev == bare_module_lexer__prev_expr) {
        i++;
        prev = bare_module_lexer__prev_op;
        continue;
      }

      err = bare_module_lexer__lex_regex(s, n, i, &i);
      if (err < 0) {
        i++;
        prev = bare_module_lexer__prev_op;
      } else {
        prev = bare_module_lexer__prev_expr;
      }
      continue;
    }

    // Numeric literal.
    if (ch >= '0' && ch <= '9') {
      while (i < n && (id(u(0)) || u(0) == '.')) i++;
      prev = bare_module_lexer__prev_expr;
      continue;
    }

    // Identifier or keyword.
    if (ids(ch) || ch >= 0x80) {
      size_t ks = i;
      while (i < n && idl(u(0))) i++;
      size_t ke = i;
      size_t kl = ke - ks;

      // Member property - never a keyword, except the '__exportStar' family
      // of helpers, which transpilers invoke through a module object
      // ('tslib_1.__exportStar(require(...), exports)').
      if (prev == bare_module_lexer__prev_dot && !bare_module_lexer__match_kw(s, ks, kl, "__export", 8) && !bare_module_lexer__match_kw(s, ks, kl, "__exportStar", 12)) {
        prev = bare_module_lexer__prev_expr;
        continue;
      }

      type = 0;
      names = NULL;
      attributes = NULL;
      type_only = false;

      if (bare_module_lexer__match_kw(s, ks, kl, "require", 7)) {
        is = ks;
        prev = bare_module_lexer__prev_expr;

        goto require;
      }

      if (bare_module_lexer__match_kw(s, ks, kl, "import", 6)) {
        type |= bare_module_lexer_import;
        is = ks;
        prev = bare_module_lexer__prev_expr;

        size_t after = bare_module_lexer__skip_trivia(s, n, i);
        bool separated = after > i;

        i = after;

        // import \*
        if (c(0) == '*') {
          i++;

          i = bare_module_lexer__skip_trivia(s, n, i);

          // import \* as
          if (bare_module_lexer__at_kw(s, n, i, "as", 2)) {
            i = bare_module_lexer__skip_trivia(s, n, i + 2);

            // import \* as [^\s]+
            while (i < n && !ws(u(0))) i++;

            i = bare_module_lexer__skip_trivia(s, n, i);

            // import \* as [^\s]+ from
            if (bare_module_lexer__at_kw(s, n, i, "from", 4)) {
              i += 4;

              err = bare_module_lexer__add_name(env, &names, &nl, (const utf8_t *) "*", -1);
              assert(err == 0);

              goto from;
            }
          }
        }

        // import {
        else if (c(0) == '{') {
          err = bare_module_lexer__lex_import_names(env, &names, &nl, s, n, i, &i, &type_only);
          if (err < 0) goto err;

          i = bare_module_lexer__skip_trivia(s, n, i);

          // import {[^}]*} from
          if (bare_module_lexer__at_kw(s, n, i, "from", 4)) {
            i += 4;

            goto from;
          }
        }

        // import ['"]
        else if (c(0) == '\'' || c(0) == '"') {
          if (bare_module_lexer__lex_string(s, n, &i, &ss, &se)) {
            i = bare_module_lexer__skip_trivia(s, n, i);

            // import ['"][^'"]*['"] with
            if (bare_module_lexer__at_kw(s, n, i, "with", 4)) {
              i += 4;

              err = bare_module_lexer__lex_import_attributes(env, &attributes, s, n, i, &i);
              if (err < 0) goto err;
            }

            err = bare_module_lexer__add_import(env, imports, &il, s, is, ss, se, type, names, attributes);
            if (err < 0) goto err;
          }
        }

        // import\(
        else if (c(0) == '(') {
          type |= bare_module_lexer_dynamic;

          err = bare_module_lexer__lex_call(env, &attributes, s, n, &i, &ss, &se, type, &matched);
          if (err < 0) goto err;

          if (matched) {
            err = bare_module_lexer__add_import(env, imports, &il, s, is, ss, se, type, names, attributes);
            if (err < 0) goto err;
          }
        }

        // import\.
        else if (c(0) == '.') {
          i++;

          i = bare_module_lexer__skip_trivia(s, n, i);

          // import\.meta
          if (bare_module_lexer__at_kw(s, n, i, "meta", 4)) {
            i += 4;

            i = bare_module_lexer__skip_trivia(s, n, i);

            // import\.meta\.(addon(\.resolve)?|asset|resolve)
            if (c(0) == '.') {
              size_t p = bare_module_lexer__lex_member_suffix(s, n, i, &type);

              if (p == i) continue;

              i = p;

              if (c(0) == '(') {
                err = bare_module_lexer__lex_call(env, &attributes, s, n, &i, &ss, &se, type, &matched);
                if (err < 0) goto err;

                if (matched) {
                  err = bare_module_lexer__add_import(env, imports, &il, s, is, ss, se, type, names, attributes);
                  if (err < 0) goto err;
                }

                // import\.meta\.addon(\.resolve)?\(\)
                else if (c(0) == ')' && (type & bare_module_lexer_addon)) {
                  ss = se = i++;

                  err = bare_module_lexer__add_import(env, imports, &il, s, is, ss, se, type, names, attributes);
                  if (err < 0) goto err;
                }
              }
            }
          }
        }

        // import [default]
        else if (separated) {
          size_t j = i;

          // import [^\s,]+
          while (i < n && !ws(u(0)) && u(0) != ',') i++;

          if (j < i) {
            i = bare_module_lexer__skip_trivia(s, n, i);

            // import [^\s,]+ from
            if (bare_module_lexer__at_kw(s, n, i, "from", 4)) {
              i += 4;

              err = bare_module_lexer__add_name(env, &names, &nl, (const utf8_t *) "default", -1);
              assert(err == 0);

              goto from;
            }

            // import [^\s,]+,
            else if (c(0) == ',') {
              i++;

              i = bare_module_lexer__skip_trivia(s, n, i);

              // import [^\s,]+, {
              if (c(0) == '{') {
                err = bare_module_lexer__add_name(env, &names, &nl, (const utf8_t *) "default", -1);
                assert(err == 0);

                err = bare_module_lexer__lex_import_names(env, &names, &nl, s, n, i, &i, NULL);
                if (err < 0) goto err;

                i = bare_module_lexer__skip_trivia(s, n, i);

                // import [^\s,]+, {[^}]*} from
                if (bare_module_lexer__at_kw(s, n, i, "from", 4)) {
                  i += 4;

                  goto from;
                }
              }

              // import [^\s,]+, \*
              else if (c(0) == '*') {
                i++;

                i = bare_module_lexer__skip_trivia(s, n, i);

                // import [^\s,]+, \* as
                if (bare_module_lexer__at_kw(s, n, i, "as", 2)) {
                  i = bare_module_lexer__skip_trivia(s, n, i + 2);

                  // import [^\s,]+, \* as [^\s]+
                  while (i < n && !ws(u(0))) i++;

                  i = bare_module_lexer__skip_trivia(s, n, i);

                  // import [^\s,]+, \* as [^\s]+ from
                  if (bare_module_lexer__at_kw(s, n, i, "from", 4)) {
                    i += 4;

                    err = bare_module_lexer__add_name(env, &names, &nl, (const utf8_t *) "default", -1);
                    assert(err == 0);

                    err = bare_module_lexer__add_name(env, &names, &nl, (const utf8_t *) "*", -1);
                    assert(err == 0);

                    goto from;
                  }
                }
              }
            }
          }
        }

        continue;
      }

      if (bare_module_lexer__match_kw(s, ks, kl, "module", 6)) {
        es = ks;
        prev = bare_module_lexer__prev_expr;

        // Peek for '.exports' or '.require' without consuming, so other
        // members ('module.id', ...) lex as plain member access.
        size_t p = bare_module_lexer__skip_trivia(s, n, ke);

        if (p < n && s[p] == '.') {
          size_t q = bare_module_lexer__skip_trivia(s, n, p + 1);

          // module\.exports
          if (bare_module_lexer__at_kw(s, n, q, "exports", 7)) {
            i = q + 7;

            goto exports;
          }

          // module\.require - behaves like a plain require.
          if (bare_module_lexer__at_kw(s, n, q, "require", 7)) {
            is = q;
            i = q + 7;

            goto require;
          }
        }

        continue;
      }

      if (bare_module_lexer__match_kw(s, ks, kl, "exports", 7)) {
        es = ks;
        prev = bare_module_lexer__prev_expr;

        goto exports;
      }

      if (bare_module_lexer__match_kw(s, ks, kl, "export", 6)) {
        es = ks;
        prev = bare_module_lexer__prev_expr;

        i = bare_module_lexer__skip_trivia(s, n, i);

        // export \*
        if (c(0) == '*') {
          i++;

          i = bare_module_lexer__skip_trivia(s, n, i);

          // export \* as
          if (bare_module_lexer__at_kw(s, n, i, "as", 2)) {
            i = bare_module_lexer__skip_trivia(s, n, i + 2);

            // export \* as [^\s]+
            while (i < n && !ws(u(0))) i++;

            i = bare_module_lexer__skip_trivia(s, n, i);

            // export \* as [^\s]+ from
            if (bare_module_lexer__at_kw(s, n, i, "from", 4)) {
              type |= bare_module_lexer_import | bare_module_lexer_reexport;
              is = es;

              i += 4;

              err = bare_module_lexer__add_name(env, &names, &nl, (const utf8_t *) "*", -1);
              assert(err == 0);

              goto from;
            }
          }

          // export \* from
          else if (bare_module_lexer__at_kw(s, n, i, "from", 4)) {
            type |= bare_module_lexer_import | bare_module_lexer_reexport;
            is = es;

            i += 4;

            err = bare_module_lexer__add_name(env, &names, &nl, (const utf8_t *) "*", -1);
            assert(err == 0);

            goto from;
          }
        }

        // export {
        else if (c(0) == '{') {
          size_t bs = ++i;

          // export {[^}]*
          while (i < n && u(0) != '}') i++;

          // export {[^}]*}
          if (c(0) == '}') {
            size_t be = i;

            i++;

            i = bare_module_lexer__skip_trivia(s, n, i);

            // export {[^}]*} from
            if (bare_module_lexer__at_kw(s, n, i, "from", 4)) {
              type |= bare_module_lexer_import | bare_module_lexer_reexport;
              is = es;

              size_t resume = i + 4;

              i = bs - 1;

              err = bare_module_lexer__lex_import_names(env, &names, &nl, s, n, i, &i, &type_only);
              if (err < 0) goto err;

              i = resume;

              goto from;
            }

            // export {[^}]*}
            size_t resume = i;

            i = bs;

            while (i < be) {
              i = bare_module_lexer__skip_trivia(s, be, i);

              // A leading `type` modifier erases the binding it precedes.
              size_t p = bare_module_lexer__lex_type_modifier(s, be, i);
              bool is_type = p != i;

              if (is_type) i = p;

              if (i < be && idsl(u(0))) {
                ss = i++;

                while (i < be && idl(u(0))) i++;

                se = i;

                i = bare_module_lexer__skip_trivia(s, be, i);

                // export {[^,}]+ as
                if (bare_module_lexer__at_kw(s, be, i, "as", 2)) {
                  i = bare_module_lexer__skip_trivia(s, be, i + 2);

                  // export {[^,}]+ as [^\s,}]+ - the alias is the exported
                  // name, either an identifier or a string literal.
                  if (i < be && idsl(u(0))) {
                    ss = i++;

                    while (i < be && idl(u(0))) i++;

                    se = i;
                  } else if (i < be && (u(0) == '\'' || u(0) == '"')) {
                    if (!bare_module_lexer__lex_string(s, be, &i, &ss, &se)) break;
                  }
                }

                // A type-only binding is erased - export no name for it.
                if (!is_type) {
                  err = bare_module_lexer__add_export(env, exports, &el, s, es, ss, se);
                  if (err < 0) goto err;
                }
              }

              i = bare_module_lexer__skip_trivia(s, be, i);

              if (c(0) != ',') break;

              i++;
            }

            i = resume;
          }
        }

        // export = require('id') - a TypeScript export assignment that
        // re-exports the required module wholesale, like 'module.exports =
        // require('id')'.
        else if (c(0) == '=') {
          i = bare_module_lexer__skip_trivia(s, n, i + 1);

          // export = require
          if (bare_module_lexer__at_kw(s, n, i, "require", 7)) {
            type |= bare_module_lexer_reexport;
            is = i;

            i += 7;

            goto require;
          }
        }

        // export import name = require('id') - a CommonJS re-export via a
        // TypeScript import assignment. The bound name is re-exported, so the
        // require is flagged as a re-export.
        else if (bare_module_lexer__at_kw(s, n, i, "import", 6)) {
          i = bare_module_lexer__skip_trivia(s, n, i + 6);

          // export import [name]
          if (i < n && idsl(u(0))) {
            while (i < n && idl(u(0))) i++;

            i = bare_module_lexer__skip_trivia(s, n, i);

            // export import [name] =
            if (c(0) == '=') {
              i = bare_module_lexer__skip_trivia(s, n, i + 1);

              // export import [name] = require
              if (bare_module_lexer__at_kw(s, n, i, "require", 7)) {
                type |= bare_module_lexer_reexport;
                is = i;

                i += 7;

                goto require;
              }
            }
          }
        }

        // export const
        else if (bare_module_lexer__at_kw(s, n, i, "const", 5)) {
          i += 5;

          goto declarations;
        }

        // export let
        else if (bare_module_lexer__at_kw(s, n, i, "let", 3)) {
          i += 3;

          goto declarations;
        }

        // export var
        else if (bare_module_lexer__at_kw(s, n, i, "var", 3)) {
          i += 3;

          goto declarations;
        }

        // export function
        else if (bare_module_lexer__at_kw(s, n, i, "function", 8)) {
          i += 8;

          i = bare_module_lexer__skip_trivia(s, n, i);

          // export function\*
          if (c(0) == '*') i++;

          goto declaration;
        }

        // export async function
        else if (bare_module_lexer__at_kw(s, n, i, "async", 5)) {
          size_t p = bare_module_lexer__skip_trivia(s, n, i + 5);

          if (bare_module_lexer__at_kw(s, n, p, "function", 8)) {
            i = bare_module_lexer__skip_trivia(s, n, p + 8);

            // export async function\*
            if (c(0) == '*') i++;

            goto declaration;
          }
        }

        // export class
        else if (bare_module_lexer__at_kw(s, n, i, "class", 5)) {
          i += 5;

          goto declaration;
        }

        // export default
        else if (bare_module_lexer__at_kw(s, n, i, "default", 7)) {
          ss = i;
          se = i + 7;

          i += 7;

          err = bare_module_lexer__add_export(env, exports, &el, s, es, ss, se);
          if (err < 0) goto err;
        }

        continue;
      }

      if (bare_module_lexer__match_kw(s, ks, kl, "__export", 8) || bare_module_lexer__match_kw(s, ks, kl, "__exportStar", 12)) {
        prev = bare_module_lexer__prev_expr;

        i = bare_module_lexer__skip_trivia(s, n, i);

        // __export(Star)?\(
        if (c(0) == '(') {
          i++;

          i = bare_module_lexer__skip_trivia(s, n, i);

          // __export(Star)?\(require
          if (bare_module_lexer__at_kw(s, n, i, "require", 7)) {
            type |= bare_module_lexer_reexport;
            is = i;

            i += 7;

            goto require;
          }
        }

        continue;
      }

      // TypeScript type alias 'type Name [<...>] = <type>'. The whole
      // declaration is a type, so it is skipped and any import(...) inside it
      // is not lexed as a module import. The alias name must sit on the same
      // line as 'type' so a plain 'type' expression before a line break isn't
      // mistaken for an alias.
      if (bare_module_lexer__match_kw(s, ks, kl, "type", 4) && prev != bare_module_lexer__prev_dot) {
        size_t p = bare_module_lexer__skip_trivia(s, n, ke);

        if (p < n && idsl(s[p]) && !bare_module_lexer__has_line_terminator(s, ke, p)) {
          size_t eq = bare_module_lexer__skip_type(s, n, p);

          // 'type Name ... =' (but not '=>') confirms a type alias.
          if (eq < n && s[eq] == '=' && !(eq + 1 < n && s[eq + 1] == '>')) {
            i = bare_module_lexer__skip_type(s, n, eq + 1);
            prev = bare_module_lexer__prev_op;

            continue;
          }
        }

        // Not an alias - fall through and treat 'type' as a plain identifier.
      }

      // TypeScript variable type annotation '(const|let|var) name: <type>'.
      // The annotation is skipped so import(...) inside it isn't lexed as a
      // module import, while an '=' initializer is left for the main loop so
      // require()/import() in it are still seen.
      if (prev == bare_module_lexer__prev_op && (bare_module_lexer__match_kw(s, ks, kl, "const", 5) || bare_module_lexer__match_kw(s, ks, kl, "let", 3) || bare_module_lexer__match_kw(s, ks, kl, "var", 3))) {
        size_t p = bare_module_lexer__skip_trivia(s, n, ke);

        if (p < n && idsl(s[p])) {
          size_t q = p + 1;

          while (q < n && idl(s[q])) q++;

          size_t r = bare_module_lexer__skip_trivia(s, n, q);

          if (r < n && s[r] == ':') {
            i = bare_module_lexer__skip_type(s, n, r + 1);
            prev = bare_module_lexer__prev_op;

            continue;
          }
        }

        // Fall through - a plain declaration lexed as ordinary code.
      }

      // Plain identifier or a keyword that only affects classification.
      prev = bare_module_lexer__classify_kw(s, ks, kl);
      continue;
    }

    // Punctuation relevant to context tracking.
    if (ch == '.') {
      // Spread '...' - an expression follows.
      if (i + 2 < n && u(1) == '.' && u(2) == '.') {
        i += 3;
        prev = bare_module_lexer__prev_op;
        continue;
      }

      i++;
      prev = bare_module_lexer__prev_dot;
      continue;
    }

    if (ch == '(') {
      paren_depth++;
      i++;
      prev = bare_module_lexer__prev_op;
      continue;
    }

    if (ch == ')') {
      if (paren_depth > 0) paren_depth--;
      i++;
      prev = bare_module_lexer__prev_expr;
      continue;
    }

    if (ch == '[') {
      bracket_depth++;
      i++;
      prev = bare_module_lexer__prev_op;
      continue;
    }

    if (ch == ']') {
      if (bracket_depth > 0) bracket_depth--;
      i++;
      prev = bare_module_lexer__prev_expr;
      continue;
    }

    if (ch == '{') {
      brace_depth++;
      i++;
      prev = bare_module_lexer__prev_op;
      continue;
    }

    if (ch == '}') {
      // The '}' closing an open template substitution resumes template
      // content scanning.
      if (template_depth > 0 && brace_depth == template_stack[template_depth - 1]) {
        template_depth--;
        i++;

        goto template;
      }

      if (brace_depth > 0) brace_depth--;
      if (declarators && brace_depth < d_brace) declarators = false;
      if (properties && brace_depth < p_brace) properties = false;
      i++;
      prev = bare_module_lexer__prev_op;
      continue;
    }

    if (ch == ',') {
      i++;
      prev = bare_module_lexer__prev_op;

      // A pending exported declarator list resumes at a ',' at its own
      // nesting depth.
      if (declarators && brace_depth == d_brace && paren_depth == d_paren && bracket_depth == d_bracket) {
        goto declarations;
      }

      // A pending 'module.exports = { ... }' resumes at a ',' at its own
      // nesting depth.
      if (properties && brace_depth == p_brace && paren_depth == p_paren && bracket_depth == p_bracket) {
        goto properties;
      }

      continue;
    }

    if (ch == ';') {
      declarators = false;
      i++;
      prev = bare_module_lexer__prev_op;
      continue;
    }

    i++;
    prev = bare_module_lexer__prev_op;
    continue;

  require:
    type |= bare_module_lexer_require;

    i = bare_module_lexer__skip_trivia(s, n, i);

    // require\.(addon(\.resolve)?|asset|resolve)
    if (c(0) == '.') {
      size_t p = bare_module_lexer__lex_member_suffix(s, n, i, &type);

      if (p == i) continue;

      i = p;
    }

    // require(\.(resolve|addon(\.resolve)?|asset))?\(
    if (c(0) == '(') {
      err = bare_module_lexer__lex_call(env, &attributes, s, n, &i, &ss, &se, type, &matched);
      if (err < 0) goto err;

      if (matched) {
        err = bare_module_lexer__add_import(env, imports, &il, s, is, ss, se, type, names, attributes);
        if (err < 0) goto err;
      }

      // require\.addon(\.resolve)?\(\)
      else if (c(0) == ')' && (type & bare_module_lexer_addon)) {
        ss = se = i++;

        err = bare_module_lexer__add_import(env, imports, &il, s, is, ss, se, type, names, attributes);
        if (err < 0) goto err;
      }
    }

    continue;

  exports:
    i = bare_module_lexer__skip_trivia(s, n, i);

    // exports =
    if (c(0) == '=') {
      i++;

      i = bare_module_lexer__skip_trivia(s, n, i);

      // exports = \{
      if (c(0) == '{') {
        i++;
        brace_depth++;
        prev = bare_module_lexer__prev_op;

        properties = true;
        p_brace = brace_depth;
        p_paren = paren_depth;
        p_bracket = bracket_depth;

        goto properties;
      }

      // exports = require
      else if (bare_module_lexer__at_kw(s, n, i, "require", 7)) {
        type |= bare_module_lexer_reexport;
        is = i;

        i += 7;

        goto require;
      }
    }

    // exports\.
    else if (c(0) == '.') {
      i++;

      i = bare_module_lexer__skip_trivia(s, n, i);

      ss = i;

      while (i < n && !ws(u(0)) && u(0) != '=') i++;

      se = i;

      i = bare_module_lexer__skip_trivia(s, n, i);

      // exports\.[^\s=] =
      if (c(0) == '=') {
        i++;

        err = bare_module_lexer__add_export(env, exports, &el, s, es, ss, se);
        if (err < 0) goto err;
      }
    }

    // exports\[
    else if (c(0) == '[') {
      i++;

      i = bare_module_lexer__skip_trivia(s, n, i);

      // exports\[['"]
      if (c(0) == '\'' || c(0) == '"') {
        if (bare_module_lexer__lex_string(s, n, &i, &ss, &se)) {
          i = bare_module_lexer__skip_trivia(s, n, i);

          while (i < n && u(0) != ']') i++;

          // exports\[['"].*['"][^\]]*\]
          if (c(0) == ']') {
            i++;

            i = bare_module_lexer__skip_trivia(s, n, i);

            // exports\[['"].*['"][^\]]*\] =
            if (c(0) == '=') {
              i++;

              err = bare_module_lexer__add_export(env, exports, &el, s, es, ss, se);
              if (err < 0) goto err;
            }
          }
        }
      }
    }

    continue;

  from:
    i = bare_module_lexer__skip_trivia(s, n, i);

    // from ['"]
    if (c(0) == '\'' || c(0) == '"') {
      if (bare_module_lexer__lex_string(s, n, &i, &ss, &se)) {
        i = bare_module_lexer__skip_trivia(s, n, i);

        if (bare_module_lexer__at_kw(s, n, i, "with", 4)) {
          i += 4;

          err = bare_module_lexer__lex_import_attributes(env, &attributes, s, n, i, &i);
          if (err < 0) goto err;
        }

        // A type-only specifier list is erased along with its declaration.
        if (!type_only) {
          err = bare_module_lexer__add_import(env, imports, &il, s, is, ss, se, type, names, attributes);
          if (err < 0) goto err;
        }
      }
    }

    continue;

  declaration:
    i = bare_module_lexer__skip_trivia(s, n, i);

    // ((async )?function\*?|class) [identifier]
    if (i < n && idsl(u(0))) {
      ss = i++;

      while (i < n && idl(u(0))) i++;

      se = i;

      err = bare_module_lexer__add_export(env, exports, &el, s, es, ss, se);
      if (err < 0) goto err;
    }

    continue;

  declarations:
    declarators = false;

    i = bare_module_lexer__skip_trivia(s, n, i);

    // (const|let|var) followed by a destructuring pattern - the bindings
    // are the exported names.
    if (c(0) == '{' || c(0) == '[') {
      err = bare_module_lexer__lex_pattern(env, exports, &el, s, n, es, &i, 0);
      if (err < 0) goto err;
    }

    // (const|let|var) followed by a plain binding.
    else if (i < n && idsl(u(0))) {
      ss = i++;

      while (i < n && idl(u(0))) i++;

      se = i;

      err = bare_module_lexer__add_export(env, exports, &el, s, es, ss, se);
      if (err < 0) goto err;
    }

    else continue;

    i = bare_module_lexer__skip_trivia(s, n, i);

    // ',' - the next declarator follows immediately.
    if (c(0) == ',') {
      i++;

      goto declarations;
    }

    // '=' - an initializer follows. Let the main loop lex it as ordinary
    // code (it may contain require() or import()) and pick the declarator
    // list back up at the next ',' at this nesting depth.
    if (c(0) == '=') {
      i++;

      declarators = true;
      d_brace = brace_depth;
      d_paren = paren_depth;
      d_bracket = bracket_depth;
    }

    continue;

  properties:
    i = bare_module_lexer__skip_trivia(s, n, i);

    // End of object - let the outer loop consume the '}', which clears the
    // properties state via brace depth tracking.
    if (i >= n || u(0) == '}') continue;

    // Stray ',' or leading ',' (comma-first style after the opening brace).
    if (u(0) == ',') {
      i++;

      goto properties;
    }

    // Identifier key: shorthand, 'name: value', or a method.
    if (idsl(u(0))) {
      ss = i++;

      while (i < n && idl(u(0))) i++;

      se = i;

      i = bare_module_lexer__skip_trivia(s, n, i);

      // 'get name()', 'async name()' - the exported name follows the
      // modifier.
      if (i < n && idsl(u(0))) {
        ss = i++;

        while (i < n && idl(u(0))) i++;

        se = i;

        i = bare_module_lexer__skip_trivia(s, n, i);
      }

      err = bare_module_lexer__add_export(env, exports, &el, s, es, ss, se);
      if (err < 0) goto err;

      // 'name:' - let the outer loop lex the value and resume at the next
      // ',' at this nesting depth.
      if (c(0) == ':') {
        i++;
        prev = bare_module_lexer__prev_op;

        continue;
      }

      // 'name(' - a method definition. Let the outer loop lex the parameter
      // list and body as ordinary code (it may contain require() or
      // import()) and resume at the next ',' at this nesting depth.
      if (c(0) == '(') {
        prev = bare_module_lexer__prev_op;

        continue;
      }

      // Shorthand - the next entry follows.
      goto properties;
    }

    // Spread of require(...) - 'module.exports = { ...require("x") }' spreads
    // the required module's exports onto 'module.exports', which is the same
    // re-export as 'module.exports = require("x")'.
    if (i + 2 < n && u(0) == '.' && u(1) == '.' && u(2) == '.') {
      size_t p = bare_module_lexer__skip_trivia(s, n, i + 3);

      if (bare_module_lexer__at_kw(s, n, p, "require", 7)) {
        type = bare_module_lexer_reexport;
        names = NULL;
        attributes = NULL;
        is = p;
        i = p + 7;
        prev = bare_module_lexer__prev_expr;

        goto require;
      }
    }

    // String key - 'module.exports = { "foo": ... }'. The string literal is a
    // statically known export name, used when the name isn't a valid
    // identifier (e.g. a kebab-cased name).
    if (u(0) == '\'' || u(0) == '"') {
      size_t ns; // Name start
      size_t ne; // Name end

      if (bare_module_lexer__lex_string(s, n, &i, &ns, &ne)) {
        i = bare_module_lexer__skip_trivia(s, n, i);

        // '"foo":' (property) or '"foo"(' (method) - both name an export.
        if (c(0) == ':' || c(0) == '(') {
          err = bare_module_lexer__add_export(env, exports, &el, s, es, ns, ne);
          if (err < 0) goto err;

          // Consume the ':' but leave the '(' for the outer loop to lex the
          // method's parameter list and body.
          if (c(0) == ':') i++;

          prev = bare_module_lexer__prev_op;

          continue;
        }
      }

      // Unterminated string or not a key. Resume at the next entry.
      prev = bare_module_lexer__prev_expr;

      continue;
    }

    // Computed key with a string literal - 'module.exports = { ["foo"]: ... }'.
    // The name is still statically known. Other computed keys (identifiers,
    // expressions, or a require()) can't be resolved and fall through to be
    // lexed as ordinary code below.
    if (u(0) == '[') {
      size_t p = bare_module_lexer__skip_trivia(s, n, i + 1);

      if (p < n && (s[p] == '\'' || s[p] == '"')) {
        size_t ns; // Name start
        size_t ne; // Name end
        size_t q = p;

        if (bare_module_lexer__lex_string(s, n, &q, &ns, &ne)) {
          q = bare_module_lexer__skip_trivia(s, n, q);

          if (q < n && s[q] == ']') {
            q = bare_module_lexer__skip_trivia(s, n, q + 1);

            // '["foo"]:' (property) or '["foo"](' (method).
            if (q < n && (s[q] == ':' || s[q] == '(')) {
              err = bare_module_lexer__add_export(env, exports, &el, s, es, ns, ne);
              if (err < 0) goto err;

              i = s[q] == ':' ? q + 1 : q;
              prev = bare_module_lexer__prev_op;

              continue;
            }
          }
        }
      }
    }

    // Numeric key, generator method, a spread of anything other than
    // require(), or any other computed key. No export name can be extracted
    // (a numeric key would require canonicalization, e.g. '0xff' names the
    // property "255"), but the entry may still contain require() or import().
    // Let the outer loop lex it as ordinary code and resume at the next ','
    // at this nesting depth.
    prev = bare_module_lexer__prev_op;

    continue;
  }

#undef u
#undef c
#undef lt
#undef ws
#undef ids
#undef id
#undef idl
#undef idsl

  return 0;

err:
  return -1;
}

#endif // BARE_MODULE_LEXER_H
