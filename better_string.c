#include "better_string.h"
#include "macros.h"
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>

const string_t EMPTY_STRING = { .value = NULL, .len = 0, .capacity = 0 };

string_t *string_from_alloc_sized(const void *src, size_t len) {
  if (len == 0) return NULL;
  string_t *s = malloc(sizeof(string_t));
  if (s == NULL) return NULL;
  s->capacity = len + 1;
  s->value = malloc(s->capacity * sizeof(byte_t));
  memcpy(s->value, src, s->capacity);
  s->len = len;
  return s;
}

string_t *string_from_with_allocator(const void *src, string_allocator_t allocator, void *state) {
  if (src == NULL) return NULL;
  size_t len = strlen(src);
  if (len == 0) return allocator(state, 0);
  string_t *s = allocator(state, len + 1);
  if (s == NULL) return NULL;
  if (s->value == NULL) {
    free(s);
    return NULL;
  }
  s->capacity = len + 1;
  s->len = len;
  memcpy(s->value, src, s->capacity);
  return s;
}

string_t *string_from_alloc(const void *src) {
  if (src == NULL) return NULL;
  return string_from_alloc_sized(src, strlen(src));
}

string_t *string_with_capacity(size_t capacity) {
  if (capacity == 0) return NULL;
  string_t *s = malloc(sizeof(string_t));
  if (s == NULL) return NULL;
  s->value = malloc(capacity * sizeof(byte_t));
  if (s->value == NULL) {
    free(s);
    return NULL;
  }
  s->value[0] = '\0';
  s->len = 0;
  s->capacity = capacity;
  return s;
}

string_t *string_default_allocator(void *state, size_t capacity) {
  return string_with_capacity(capacity);
}

void string_default_deallocator(void *state, string_t *string) {
  string_free(string);
}

string_t *string_alloc(const string_t s) {
  string_t *ns = malloc(sizeof(string_t));
  if (ns) *ns = s;
  return ns;
}

int utf32_to_utf8(byte_t buf[4], unsigned long code) {
  if (code == 0) {
    return 0;
  }
  if (code < 0x80) {
    buf[0] = code;
    return 1;
  }
  if (code < 0x800) {
    buf[0] = 0xC0 | (code >> 6);            /* 110xxxxx */
    buf[1] = 0x80 | (code & 0x3F);          /* 10xxxxxx */
    return 2;
  }
  if (code < 0x10000) {
    buf[0] = 0xE0 | (code >> 12);           /* 1110xxxx */
    buf[1] = 0x80 | ((code >> 6) & 0x3F);   /* 10xxxxxx */
    buf[2] = 0x80 | (code & 0x3F);          /* 10xxxxxx */
    return 3;
  }
  if (code < 0x110000) {
    buf[0] = 0xF0 | (code >> 18);           /* 11110xxx */
    buf[1] = 0x80 | ((code >> 12) & 0x3F);  /* 10xxxxxx */
    buf[2] = 0x80 | ((code >> 6) & 0x3F);   /* 10xxxxxx */
    buf[3] = 0x80 | (code & 0x3F);          /* 10xxxxxx */
    return 4;
  }
  return -1;
}

int32_t utf8_to_utf32(const byte_t *u) {
  if (u[0] < 0xC0) return u[0];
  if (u[0] < 0xE0) return (u[0] - 0xC0) * 0x40 + u[1] - 0x80;
  if (u[0] < 0xF0) return (u[0] - 0xE0) * 0x1000 + (u[1] - 0x80) * 0x40 + u[2] - 0x80;
  int32_t code = (u[0] - 0xF0) * 0x40000 + (u[1] - 0x80) * 1000 + (u[2] - 0x80) * 0x40 + u[3] - 0x80;
  if (code >= 0x110000) return -1;
  return code;
}

bool valid_utf8(const byte_t *u) {
  if (u == NULL) return true;
  return *u < 0x80 || *u >= 0xC0;
}

int len_utf8(const byte_t *u) {
  if (u == NULL) return 0;
  if (*u < 0xC0) return 1;
  if (*u < 0xE0) return 2;
  if (*u < 0xF0) return 3;
  return 4;
}

void inc_utf8(const byte_t **u) {
  *u += len_utf8(*u);
}
void dec_utf8(const byte_t **u) {
  do {
    (*u)--;
  } while (0x80 <= **u && **u < 0xC0);
}

void copy_utf8(const byte_t *u, byte_t buf[4]) {
  int len = len_utf8(u);
  buf[0] = buf[1] = buf[2] = buf[3] = '\0';
  memcpy(buf, u, len * sizeof(byte_t));
}

void fprint_utf8(FILE *f, const byte_t *u) {
  byte_t array[5] = { '\0', '\0', '\0', '\0', '\0' };
  copy_utf8(u, array);
  fprintf(f, "%s", array);
}

void print_utf8(const byte_t *u) {
  fprint_utf8(stdout, u);
}

int utf8cmp(const byte_t *u1, const byte_t *u2) {
  for (int len = len_utf8(u1); len > 1; len--) {
    if (*u1 != *u2) return *u2 - *u1;
    u1++; u2++;
  }
  return *u2 - *u1;
}

// only works on ascii, case insensitive
int valuecmp(const char *v1, const char *v2) {
  if (*v1 == '\0') {
    if (*v2 == '\0') return 0;
    return -1;
  }
  if (*v2 == '\0') return 1;
  if (toupper(*v1) == toupper(*v2)) return valuecmp(v1 + 1, v2 + 1);
  return (int)toupper(*v1) - (int)toupper(*v2);
}

int string_comp(const string_t *s1, const string_t *s2) {
  if (s1 == NULL) {
    if (s2 == NULL) return 0;
    if (s2->len == 0) return 0;
    return -1;
  }
  if (s2 == NULL) {
    if (s1->len == 0) return 0;
    return 1;
  }
  return valuecmp((char *)s1->value, (char *)s2->value);
}

size_t string_len_utf8(const string_t *s) {
  if (s == NULL) return 0;
  size_t count = 0;
  for (const byte_t *b = s->value; *b != '\0'; inc_utf8(&b))
    count++;
  return count;
}

size_t string_length(const string_t *s) {
  if (s == NULL) return 0;
  return s->len;
}

size_t string_capacity(const string_t *s) {
  if (s == NULL) return 0;
  return s->capacity;
}

string_result_t string_realloc(string_t *s, size_t size) {
  if (s == NULL) return STRING_NULL;
  byte_t *tmpvalue = realloc(s->value, size * sizeof(byte_t));
  if (tmpvalue == NULL) return STRING_MEM;
  s->value = tmpvalue;
  s->capacity = size;
  return STRING_OK;
}

string_result_t string_realloc_with_allocator(string_t *s, size_t size, string_allocator_t allocator, string_deallocator_t deallocator, void *state) {
  if (s == NULL) return STRING_NULL;
  if (s->capacity >= size) return STRING_OK;
  string_t *new_s = allocator(state, size);
  if (new_s == NULL) return STRING_MEM;
  if (new_s->value == NULL) {
    free(new_s);
    return STRING_MEM;
  }
  memcpy(new_s->value, s->value, (s->len + 1) * sizeof(byte_t));
  new_s->len = s->len;
  string_t tmpstr = *new_s;
  *new_s = *s;
  *s = tmpstr;
  deallocator(state, new_s);
  return STRING_OK;
}

string_result_t string_ensure_space_get_capacity(string_t *s, size_t n, size_t *capacity) {
  if (s == NULL) return STRING_NULL;
  if (SIZE_MAX - s->len - 1 <= n) return STRING_OVERFLOW;
  *capacity = s->capacity;
  if (s->capacity > s->len + n) return STRING_OK;
  while (*capacity <= s->len + n) {
    if (SIZE_MAX - *capacity <= *capacity) {
      *capacity = SIZE_MAX;
      break;
    }
    *capacity *= 2;
  }
  return STRING_OK;
}

string_result_t string_ensure_space_alloc(string_t *s, size_t n) {
  size_t capacity;
  RET_IF(string_ensure_space_get_capacity(s, n, &capacity));
  return string_realloc(s, capacity);
}

string_result_t string_ensure_space_with_allocator(string_t *s, size_t n, string_allocator_t allocator, string_deallocator_t deallocator, void *state) {
  size_t capacity;
  RET_IF(string_ensure_space_get_capacity(s, n, &capacity));
  return string_realloc_with_allocator(s, capacity, allocator, deallocator, state);
}

string_result_t string_ensure_capacity_alloc(string_t **s, size_t capacity) {
  if (s == NULL) return STRING_NULL;
  if (*s == NULL) {
    *s = string_with_capacity(capacity);
    return STRING_OK;
  }
  return string_realloc(*s, capacity);
}

string_result_t string_ensure_capacity_with_allocator(string_t **s, size_t capacity, string_allocator_t allocator, string_deallocator_t deallocator, void *state) {
  if (s == NULL) return STRING_NULL;
  if (*s == NULL) {
    *s = allocator(state, capacity);
    return STRING_OK;
  }
  return string_realloc_with_allocator(*s, capacity, allocator, deallocator, state);
}

string_t *string_copy_with_allocator(const string_t *s, string_allocator_t allocator, void *state) {
  if (s == NULL) return NULL;
  if (s->len >= s->capacity)
    die("catastrophic string management failure");
  if (s->len == 0) return allocator(state, 0);
  string_t *cp = allocator(state, s->len + 1);
  if (cp == NULL) return NULL;
  if (cp->value == NULL) {
    free(cp);
    return NULL;
  }
  cp->len = s->len;
  memcpy(cp->value, s->value, (cp->len + 1) * sizeof(byte_t));
  return cp;
}

string_t *string_copy_alloc(const string_t *s) {
  return string_copy_with_allocator(s, string_default_allocator, NULL);
}

string_result_t string_concat_with_allocator(string_t *s1, const string_t *s2, string_allocator_t allocator, string_deallocator_t deallocator, void *state) {
  if (s1 == NULL || s2 == NULL) return STRING_NULL;
  return string_append_n_with_allocator(s1, s2->value, s2->len, allocator, deallocator, state);
}

string_result_t string_concat_alloc(string_t *s1, string_t *s2) {
  if (s1 == NULL || s2 == NULL) return STRING_NULL;
  return string_append_n_alloc(s1, s2->value, s2->len);
}

string_result_t string_append_with_allocator(string_t *s, const byte_t *u, string_allocator_t allocator, string_deallocator_t deallocator, void *state) {
  return string_append_n_with_allocator(s, u, len_utf8(u), allocator, deallocator, state);
}

string_result_t string_append_alloc(string_t *s, const byte_t *u) {
  return string_append_n_alloc(s, u, len_utf8(u));
}

string_result_t string_append_n_with_allocator(string_t *s, const byte_t *value, size_t n, string_allocator_t allocator, string_deallocator_t deallocator, void *state) {
  if (s == NULL) return STRING_NULL;
  RET_IF(string_ensure_space_with_allocator(s, n, allocator, deallocator, state));
  memcpy(s->value + s->len, value, n * sizeof(byte_t));
  s->len += n;
  s->value[s->len] = '\0';
  return STRING_OK;
}

string_result_t string_append_n_alloc(string_t *s, const byte_t *value, size_t n) {
  if (s == NULL) return STRING_NULL;
  if (n == 0) return STRING_OK;
  RET_IF(string_ensure_space_alloc(s, n));
  memcpy(s->value + s->len, value, n * sizeof(byte_t));
  s->len += n;
  s->value[s->len] = '\0';
  return STRING_OK;
}

string_result_t string_append_all_with_allocator(string_t *s, const byte_t *value, string_allocator_t allocator, string_deallocator_t deallocator, void *state) {
  return string_append_n_with_allocator(s, value, strlen((char *)value), allocator, deallocator, state);
}

string_result_t string_append_all_alloc(string_t *s, const byte_t *value) {
  return string_append_n_alloc(s, value, strlen((char *)value));
}

string_result_t string_prepend_with_allocator(string_t *s, const byte_t *u, string_allocator_t allocator, string_deallocator_t deallocator, void *state) {
  return string_prepend_n_with_allocator(s, u, len_utf8(u), allocator, deallocator, state);
}

string_result_t string_prepend_alloc(string_t *s, const byte_t *u) {
  return string_prepend_n_alloc(s, u, len_utf8(u));
}

string_result_t string_prepend_n_with_allocator(string_t *s, const byte_t *value, size_t n, string_allocator_t allocator, string_deallocator_t deallocator, void *state) {
  if (s == NULL) return STRING_NULL;
  RET_IF(string_ensure_space_with_allocator(s, n, allocator, deallocator, state));
  for (byte_t *b = s->value + s->len; b >= s->value; b--) b[n] = b[0];
  memcpy(s->value, value, n * sizeof(byte_t));
  s->len += n;
  return STRING_OK;
}

string_result_t string_prepend_n_alloc(string_t *s, const byte_t *value, size_t n) {
  if (s == NULL) return STRING_NULL;
  if (n == 0) return STRING_OK;
  RET_IF(string_ensure_space_alloc(s, n));
  for (byte_t *b = s->value + s->len; b >= s->value; b--) b[n] = b[0];
  memcpy(s->value, value, n * sizeof(byte_t));
  s->len += n;
  return STRING_OK;
}

string_result_t string_prepend_all_with_allocator(string_t *s, const byte_t *value, string_allocator_t allocator, string_deallocator_t deallocator, void *state) {
  return string_prepend_n_with_allocator(s, value, strlen((char *)value), allocator, deallocator, state);
}

string_result_t string_prepend_all_alloc(string_t *s, const byte_t *value) {
  return string_prepend_n_alloc(s, value, strlen((char *)value));
}

void string_reverse_unchecked(string_t *s, byte_t *buf) {
  if (s == NULL) return;
  if (s->len == 0) return;
  const byte_t *sptr = s->value + s->len;
  byte_t *bufptr = buf;
  while (sptr != s->value) {
    dec_utf8(&sptr);
    copy_utf8(sptr, bufptr);
    inc_utf8((const byte_t **)&bufptr);
  }
  memcpy(s->value, buf, s->len * sizeof(byte_t));
}

void string_reverse_with_allocator(string_t *s, string_allocator_t allocator, string_deallocator_t deallocator, void *state) {
  if (string_length(s) == 0) return;
  string_t *sbuf = allocator(state, s->len);
  string_reverse_unchecked(s, sbuf->value);
  sbuf->value[0] = '\0';
  deallocator(state, sbuf);
}

bool string_contains(const string_t *s, const byte_t u[]) {
  if (s == NULL) return false;
  for (const byte_t *cptr = s->value; *cptr != '\0'; inc_utf8(&cptr)) {
    if (!utf8cmp(cptr, u)) return true;
  }
  return false;
}

void print(const string_t *s) {
  fprint(stdout, s);
}

void fprint(FILE *f, const string_t *s) {
  if (string_length(s) == 0) return;
  fprintf(f, "%s", s->value);
}

void print_pretty(const string_t *s) {
  fprint_pretty(stdout, s);
}

void fprint_pretty(FILE *f, const string_t *s) {
  if (string_length(s) == 0) return;
  for (byte_t *cptr = s->value; cptr[0] != '\0'; cptr += len_utf8(cptr)) {
    switch (cptr[0]) {
    case '\n':
      fprintf(f, "\\n");
      break;
    case '\t':
      fprintf(f, "\\t");
      break;
    case '\r':
      fprintf(f, "\\r");
      break;
    case '\'':
      fprintf(f, "\\'");
      break;
    case '\\':
      fprintf(f, "\\\\");
      break;
    case '\a':
      fprintf(f, "\\a");
      break;
    case '\b':
      fprintf(f, "\\b");
      break;
    case '\e':
      fprintf(f, "\\e");
      break;
    case '\f':
      fprintf(f, "\\f");
      break;
    case '\v':
      fprintf(f, "\\v");
      break;
    default:
      char buf[5] = { '\0' };
      memcpy(buf, cptr, len_utf8(cptr));
      fprintf(f, "%s", buf);
    }
  }
}

void print_term(const string_t *s) {
  fprint_term(stdout, s, isatty(STDOUT_FILENO));
}

void fprint_term(FILE *f, const string_t *s, bool isterm) {
  if (string_length(s) == 0) return;
  if (isterm)
    fprint_pretty(f, s);
  else
    fprintf(f, "%s", s->value);
}

void file_read2buf(FILE *f, string_t *s) {
  if (s == NULL) return;
  s->len = fread(s->value, sizeof(byte_t), s->capacity, f);
  s->value[s->len] = '\0';
}

string_t *file_read_with_allocator(FILE *f, string_allocator_t allocator, void *state) {
  fseek(f, 0, SEEK_END);
  size_t len = ftell(f);
  fseek(f, 0, SEEK_SET);
  string_t *s = allocator(state, len + 1);
  if (s == NULL) return NULL;
  if (s->value == NULL) {
    free(s);
    return NULL;
  }
  file_read2buf(f, s);
  return s;
}

string_t *file_read_alloc(FILE *f) {
  return file_read_with_allocator(f, string_default_allocator, NULL);
}

string_t *file_read_line_with_allocator(FILE *f, string_allocator_t allocator, string_deallocator_t deallocator, void *state) {
  string_t *s = allocator(state, DEFAULT_STRING_LENGTH);
  if (s == NULL) return NULL;
  if (s->value == NULL) {
    free(s);
    return NULL;
  }
  byte_t b = fgetc(f);
  while (b != '\0') {
    if (string_append_with_allocator(s, &b, allocator, deallocator, state))
      return s;
    if (b == '\n') break;
    b = fgetc(f);
  }
  return s;
}

string_t *file_read_line_alloc(FILE *f) {
  string_t *s = string_with_capacity(DEFAULT_STRING_LENGTH);
  if (s == NULL) return NULL;
  char b = fgetc(f);
  while (b != '\0' && b != EOF) {
    if (string_append_alloc(s, (byte_t *)&b))
      return s;
    if (b == '\n') break;
    b = fgetc(f);
  }
  return s;
}

void string_empty(string_t *s) {
  if (s == NULL) return;
  s->len = 0;
}

void string_free_value(string_t *s) {
  if (s == NULL) return;
  free(s->value);
  *s = EMPTY_STRING;
}

void string_free(void *s) {
  if (s == NULL) return;
  free(((string_t *)s)->value);
  free(s);
}
