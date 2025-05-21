#ifndef LIBRARY_H_
#define LIBRARY_H_
#include "better_string.h"

typedef void(*freefunc_t)(void *);

typedef struct {
  void **values;
  size_t size;
  size_t capacity;
} stack_t;

typedef struct {
  string_t *title;
  string_t *subtitle;
  stack_t *authors;
  string_t *publisher;
  string_t *location;
  int year;
  stack_t *categories;
  bool removed;
} book_t;

extern const book_t DEFAULT_BOOK;

typedef struct BOOKNODE_STRUCT {
  struct BOOKNODE_STRUCT *next;
  book_t book;
} booknode_t;

typedef struct BOOKSLL_STRUCT {
  booknode_t *head;
} booksll_t;

typedef struct {
  enum {
    KEY_STRING,
    KEY_INT
  } type;
  union {
    string_t *key;
    int ikey;
  };
} key_t;

typedef struct AVL_STRUCT {
  struct AVL_STRUCT *left;
  struct AVL_STRUCT *right;
  uintptr_t height;
  key_t key;
  stack_t *data;
  void(*freefunc)(void *);
} avl_t;

typedef struct {
  booksll_t booklist;
  avl_t *titles;
  avl_t *subtitles;
  avl_t *authors;
  avl_t *authors_by_last_name;
  avl_t *author_last_names;
  avl_t *author_first_names;
  avl_t *publishers;
  avl_t *categories;
  avl_t *years;
  avl_t *locations;
} catalogue_t;

typedef struct {
  enum {
    LIB_OK,
    LIB_NOFILE,
    LIB_INVFILE,
    LIB_INVFORMAT,
    LIB_UNRECOVERABLE
  } status;
  catalogue_t *catalogue;
} library_t;

void stack_realloc(stack_t *s, size_t capacity);

stack_t *stack_init(size_t capacity);

void stack_push(stack_t *s, void *v);

void *stack_pop(stack_t *s);

void *stack_popdeep(stack_t *s, size_t index);

void *stack_peek(const stack_t *s);

void stack_extend(stack_t *s1, stack_t *s2);

void stack_empty(stack_t *s, freefunc_t freefunc);

bool stack_exists(stack_t *s, void *e, bool(*compfunc)(void *, void *));

size_t stack_size(const stack_t *s);

size_t stack_capacity(const stack_t *s);

void stack_free(stack_t *s, freefunc_t freefunc);

library_t lib_nofile();

library_t lib_invfile();

library_t lib_invformat();

library_t lib_unrecoverable();

book_t default_book();

void book_free(book_t b);

void trunc_string(string_t *s);

void print_book(const book_t *book);

bool read_book(book_t *bookptr);

bool book_read_from_file(FILE *f, book_t *book);

booknode_t *booknode_init();

void booknode_free(booknode_t *bn);

void booknode_print_all_books(booknode_t *bn);

void write_book_to_file(const book_t *book, FILE *f);

void booknode_write_all_to_file(booknode_t *bn, FILE *f);

bool booknode_isbook(void *bn, void *);

bool book_exists(stack_t *s);

booknode_t *booksll_add_book(booksll_t *booksll, book_t book);

void remove_book(booknode_t *bn);

catalogue_t *catalogue_init();

void catalogue_add_book(catalogue_t *c, book_t book);

void catalogue_free(catalogue_t *c);

void catalogue_print_all_books(catalogue_t *c);

void catalogue_print_all_titles(catalogue_t *c);

void catalogue_print_all_subtitles(catalogue_t *c);

void catalogue_print_all_authors(catalogue_t *c);

void catalogue_print_all_authors_by_last_name(catalogue_t *c);

void catalogue_print_all_author_last_names(catalogue_t *c);

void catalogue_print_all_author_first_names(catalogue_t *c);

void catalogue_print_all_publishers(catalogue_t *c);

void catalogue_print_all_years(catalogue_t *c);

void catalogue_print_all_categories(catalogue_t *c);

void catalogue_print_all_locations(catalogue_t *c);

void catalogue_write_to_file(catalogue_t *c, string_t *filename);

int catalogue_read_from_file(catalogue_t *c, string_t *filename);

void catalogue_search(catalogue_t *c);

void catalogue_search_avl(const avl_t *avl);

void print_search_help();

void print_help();

#endif // LIBRARY_H_
