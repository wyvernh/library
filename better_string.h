#ifndef BETTER_STRING_H_
#define BETTER_STRING_H_
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

#define DEFAULT_STRING_LENGTH 24

typedef unsigned char byte_t;

typedef struct STRING_STRUCT {
  byte_t *value;
  size_t len;
  size_t capacity;
} string_t;

typedef enum {
  STRING_OK = 0,
  STRING_MEM,
  STRING_OVERFLOW,
  STRING_NULL
} string_result_t;

typedef string_t *(*string_allocator_t)(void *state, size_t capacity);
typedef void (*string_deallocator_t)(void *state, string_t *string);

extern const string_t EMPTY_STRING;

string_t *string_from_with_allocator(const void *src, string_allocator_t allocator, void *state);

string_t *string_from_alloc(const void *src);

string_t *string_with_capacity(size_t capacity);

string_t *string_default_allocator(void *state, size_t capacity);

void string_default_deallocator(void *state, string_t *string);

string_t *string_alloc(const string_t s);

int utf32_to_utf8(byte_t buf[4], unsigned long code);

int32_t utf8_to_utf32(const byte_t *u);

bool valid_utf8(const byte_t *u);

int len_utf8(const byte_t *u);

void inc_utf8(const byte_t **u);

void dec_utf8(const byte_t **u);

void copy_utf8(const byte_t *u, byte_t buf[4]);

void fprint_utf8(FILE *f, const byte_t *u);

void print_utf8(const byte_t *u);

int utf8cmp(const byte_t *u1, const byte_t *u2);

int string_comp(const string_t *s1, const string_t *s2);

size_t string_len_utf8(const string_t *s);

size_t string_length(const string_t *s);

size_t string_capacity(const string_t *s);

string_result_t string_realloc(string_t *s, size_t size);

string_result_t string_realloc_with_allocator(string_t *s, size_t size, string_allocator_t allocator, string_deallocator_t deallocator, void *state);

string_result_t string_ensure_space_get_capacity(string_t *s, size_t n, size_t *capacity);

string_result_t string_ensure_space_alloc(string_t *s, size_t n);

string_result_t string_ensure_space_with_allocator(string_t *s, size_t n, string_allocator_t allocator, string_deallocator_t deallocator, void *state);

string_result_t string_ensure_capacity_alloc(string_t **s, size_t capacity);

string_result_t string_ensure_capacity_with_allocator(string_t **s, size_t capacity, string_allocator_t allocator, string_deallocator_t deallocator, void *state);

string_t *string_copy_with_allocator(const string_t *s, string_allocator_t allocator, void *state);

string_t *string_copy_alloc(const string_t *s);

string_result_t string_concat_with_allocator(string_t *s1, const string_t *s2, string_allocator_t allocator, string_deallocator_t deallocator, void *state);

string_result_t string_concat_alloc(string_t *s1, string_t *s2);

string_result_t string_append_with_allocator(string_t *s, const byte_t *u, string_allocator_t allocator, string_deallocator_t deallocator, void *state);

string_result_t string_append_alloc(string_t *s, const byte_t *u);

string_result_t string_append_n_with_allocator(string_t *s, const byte_t *value, size_t n, string_allocator_t allocator, string_deallocator_t deallocator, void *state);

string_result_t string_append_n_alloc(string_t *s, const byte_t *value, size_t n);

string_result_t string_append_all_with_allocator(string_t *s, const byte_t *value, string_allocator_t allocator, string_deallocator_t deallocator, void *state);

string_result_t string_append_all_alloc(string_t *s, const byte_t *value);

string_result_t string_prepend_with_allocator(string_t *s, const byte_t *u, string_allocator_t allocator, string_deallocator_t deallocator, void *state);

string_result_t string_prepend_alloc(string_t *s, const byte_t *u);

string_result_t string_prepend_n_with_allocator(string_t *s, const byte_t *value, size_t n, string_allocator_t allocator, string_deallocator_t deallocator, void *state);

string_result_t string_prepend_n_alloc(string_t *s, const byte_t *value, size_t n);

string_result_t string_prepend_all_with_allocator(string_t *s, const byte_t *value, string_allocator_t allocator, string_deallocator_t deallocator, void *state);

string_result_t string_prepend_all_alloc(string_t *s, const byte_t *value);

void string_reverse_unchecked(string_t *s, byte_t *buf);

void string_reverse_with_allocator(string_t *s, string_allocator_t allocator, string_deallocator_t deallocator, void *state);

bool string_contains(const string_t *s, const byte_t u[]);

void print(const string_t *s);

void fprint(FILE *f, const string_t *s);

void print_pretty(const string_t *s);

void fprint_pretty(FILE *f, const string_t *s);

void print_term(const string_t *s);

void fprint_term(FILE *f, const string_t *s, bool isterm);

void file_read2buf(FILE *f, string_t *s);

string_t *file_read_with_allocator(FILE *f, string_allocator_t allocator, void *state);

string_t *file_read_alloc(FILE *f);

string_t *file_read_line_with_allocator(FILE *f, string_allocator_t allocator, string_deallocator_t deallocator, void *state);

string_t *file_read_line_alloc(FILE *f);

void string_empty(string_t *s);

void string_free_value(string_t *s);

void string_free(void *s);

#endif // BETTER_STRING_H_
