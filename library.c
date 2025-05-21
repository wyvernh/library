#include <string.h>
#include "library.h"
#include "tree.h"
#include "macros.h"

const book_t DEFAULT_BOOK = {
  .title = NULL,
  .subtitle = NULL,
  .authors = NULL,
  .publisher = NULL,
  .location = NULL,
  .year = 0,
  .categories = NULL,
  .removed = false
};

void stack_realloc(stack_t *s, size_t capacity) {
  if(s->capacity >= capacity) return;
  void **values = realloc(s->values, capacity * sizeof(void *));
  if (values == NULL) die("out of memory");
  s->capacity = capacity;
  s->values = values;
}

stack_t *stack_init(size_t capacity) {
  stack_t *s = malloc(sizeof(stack_t));
  if (s == NULL) die("out of memory");
  s->values = NULL;
  if (capacity != 0) {
    s->values = malloc(capacity * sizeof(void *));
    if (s->values == NULL) die("out of memory");
  }
  s->size = 0;
  s->capacity = capacity;
  return s;
}

void stack_push(stack_t *s, void *v) {
  if (s == NULL) die("null stack");
  if (s == NULL || !v) return;
  if (s->size >= s->capacity) {
    if (SIZE_MAX - s->size < s->size + 1)
      die("stack overflow");
    stack_realloc(s, s->size * 2 + 1);
  }
  s->values[s->size] = v;
  s->size++;
}

void *stack_pop(stack_t *s) {
  if (s == NULL) die("null stack");
  if (s->size == 0) return NULL;
  s->size--;
  return s->values[s->size];
}

void *stack_popdeep(stack_t *s, size_t index) {
  if (s == NULL) die("null stack");
  if (index >= s->size) return NULL;
  void *retval = s->values[index];
  s->size--;
  for (size_t i = index; i < s->size; i++) {
    s->values[i] = s->values[i + 1];
  }
  return retval;
}

void *stack_peek(const stack_t *s) {
  if (s == NULL) die("null stack");
  if (s->size == 0) return NULL;
  return s->values[s->size - 1];
}

void stack_extend(stack_t *s1, stack_t *s2) {
  if (s1 == NULL) die("null stack");
  if (s2 == NULL) return;
  for (int i = 0; i < s2->size; i++) {
    stack_push(s1, s2->values[i]);
  }
  s2->size = 0;
  stack_free(s2, nofree);
}

void stack_empty(stack_t *s, void(*freefunc)(void *)) {
  if (s == NULL) return;
  for (size_t i = 0; i < s->size; i++) {
    freefunc(s->values[i]);
  }
  s->size = 0;
}

bool stack_exists(stack_t *s, void *e, bool(*compfunc)(void *, void *)) {
  if (s == NULL) return false;
  for (int i = 0; i < s->size; i++)
    if (compfunc(s->values[i], e)) return true;
  return false;
}

size_t stack_size(const stack_t *s) {
  if (s == NULL) die("stack null");
  return s->size;
}

size_t stack_capacity(const stack_t *s) {
  if (s == NULL) die("stack null");
  return s->capacity;
}

void stack_free(stack_t *s, freefunc_t freefunc) {
  if (s == NULL) return;
  stack_empty(s, freefunc);
  free(s->values);
  free(s);
}

void string_stack_free(stack_t *s) {
  stack_free(s, (freefunc_t)string_free);
}

library_t lib_nofile() {
  library_t lib = { LIB_NOFILE, NULL };
  return lib;
}

library_t lib_invfile() {
  library_t lib = { LIB_INVFILE, NULL };
  return lib;
}

library_t lib_invformat() {
  library_t lib = { LIB_INVFORMAT, NULL };
  return lib;
}

library_t lib_unrecoverable() {
  library_t lib = { LIB_UNRECOVERABLE, NULL };
  return lib;
}

book_t default_book() {
  book_t book = DEFAULT_BOOK;
  book.authors = stack_init(0);
  book.categories = stack_init(0);
  return book;
}

void book_free(book_t b) {
  string_free(b.title);
  string_free(b.subtitle);
  stack_free(b.authors, (freefunc_t)string_stack_free);
  string_free(b.publisher);
  string_free(b.location);
  string_stack_free(b.categories);
}

void print_authors(stack_t *s) {
  for (size_t auth = 0; auth < stack_size(s); auth++) {
    stack_t *authstack = s->values[auth];
    if (stack_size(authstack) == 0) continue;
    for (size_t name = 0; name < authstack->size - 1; name++) {
      print(authstack->values[name]);
      printf(" ");
    }
    print(authstack->values[authstack->size - 1]);
    if (auth != stack_size(s) - 1) printf(", ");
  }
}

void print_categories(stack_t *cat) {
  if (stack_size(cat) > 0) {
    printf("\nCategories: ");
    for (size_t i = 0; i < cat->size - 1; i++) {
      print(cat->values[i]);
      printf(", ");
    }
    print(cat->values[cat->size - 1]);
  }
}

void print_book(const book_t *book) {
  if (book == NULL) die("print_book(): book was null");
  if (book->removed) return;
  printf("    Title: ");
  print(book->title);
  printf("\n Subtitle: ");
  print(book->subtitle);
  printf("\nAuthor(s): ");
  print_authors(book->authors);
  printf("\nPublisher: ");
  print(book->publisher);
  printf("\n     Year: %d", book->year);
  printf("\n Location: ");
  print(book->location);
  print_categories(book->categories);
  printf("\n");
}

void trunc_string(string_t *s) {
  if (s == NULL) return;
  if (s->len == 0) return;
  s->len--;
  s->value[s->len] = '\0';
}

void parse_authors(book_t *book) {
  string_t *line = file_read_line_alloc(stdin);
  stack_t *author = stack_init(3);
  string_t *name = string_with_capacity(DEFAULT_STRING_LENGTH);
  const byte_t *b = line->value;
  while (*b == ' ' || *b == ',') b++;
  while (*b != '\0' && *b != '\n') {
    string_append_alloc(name, b);
    inc_utf8(&b);
    if (*b == ' ' || *b == ',') {
      stack_push(author, name);
      while (*b == ' ') b++;
      name = string_with_capacity(DEFAULT_STRING_LENGTH);
    }
    if (*b == ',') {
      stack_push(book->authors, author);
      while (*b == ' ' || *b == ',') b++;
      author = stack_init(3);
    }
  }
  if (string_length(name) > 0)
    stack_push(author, name);
  else
    string_free(name);
  if (stack_size(author) > 0)
    stack_push(book->authors, author);
  else
    string_stack_free(author);
  string_free(line);
}

void parse_year(book_t *book) {
  while (true) {
    string_t *s = file_read_line_alloc(stdin);
    if (sscanf((char *)s->value, "%d\n", &book->year) == 1) {
      string_free(s);
      break;
    }
    printf("Please enter a valid integer\n");
    printf("     Year: ");
    string_free(s);
  }
}

void parse_categories(book_t *book) {
  string_t *categories = file_read_line_alloc(stdin);
  if (categories->value[0] == '\n') {
    string_free(categories);
    return;
  }
  string_t *category = string_with_capacity(DEFAULT_STRING_LENGTH);
  const byte_t *b = categories->value;
  while (*b == ' ') b++;
  while (*b != '\n' && *b != '\0') {
    string_append_alloc(category, b);
    inc_utf8(&b);
    if (*b == ' ') {
      stack_push(book->categories, category);
      category = string_with_capacity(DEFAULT_STRING_LENGTH);
    }
    while (*b == ' ') b++;
  }
  if (string_length(category) > 0)
    stack_push(book->categories, category);
  else
    string_free(category);
  string_free(categories);
}

bool read_book(book_t *bookptr) {
  book_t book = default_book();
  printf("    Title: ");
  book.title = file_read_line_alloc(stdin);
  trunc_string(book.title);
  if (book.title->len == 0) {
    book_free(book);
    return false;
  }
  printf(" Subtitle: ");
  book.subtitle = file_read_line_alloc(stdin);
  trunc_string(book.subtitle);
  printf("Author(s): ");
  parse_authors(&book);
  printf("Publisher: ");
  book.publisher = file_read_line_alloc(stdin);
  trunc_string(book.publisher);
  printf("     Year: ");
  parse_year(&book);
  printf(" Location: ");
  book.location = file_read_line_alloc(stdin);
  trunc_string(book.location);
  printf("Categories: ");
  parse_categories(&book);
  *bookptr = book;
  return true;
}

bool read_next_section(const byte_t **b, string_t **s) {
  *s = string_with_capacity(DEFAULT_STRING_LENGTH);
  while (**b != ';') {
    if (**b == '\n' || **b == '\0') {
      printf("Warning: read invalid book\n");
      return true;
    }
    string_append_alloc(*s, *b);
    inc_utf8(b);
  }
  inc_utf8(b);
  return false;
}

bool read_authors(const byte_t **b, stack_t *authors) {
  stack_t *author = stack_init(3);
  string_t *name = string_with_capacity(DEFAULT_STRING_LENGTH);
  while (**b == ' ' || **b == ',') (*b)++;
  while (**b != ';') {
    if (**b == '\n' || **b == '\0') {
      printf("Warning: read invalid book\n");
      return true;
    }
    string_append_alloc(name, *b);
    inc_utf8(b);
    if (**b == ' ' || **b == ',') {
      stack_push(author, name);
      while (**b == ' ') (*b)++;
      name = string_with_capacity(DEFAULT_STRING_LENGTH);
    }
    if (**b == ',') {
      stack_push(authors, author);
      while (**b == ' ' || **b == ',') (*b)++;
      author = stack_init(3);
    }
  }
  inc_utf8(b);
  if (string_length(name) > 0)
    stack_push(author, name);
  else
    string_free(name);
  if (stack_size(author) > 0)
    stack_push(authors, author);
  else
    string_stack_free(author);
  return false;
}

#define RETURN_FALSE(result)                                      \
  if (result) { book_free(book); string_free(s); return false; }

bool book_read_from_file(FILE *f, book_t *bookptr) {
  string_t *s = file_read_line_alloc(f);
  if (s == NULL) die("book_read_from_file(): out of memory");
  if (s->value[0] == '\0' || s->value[0] == '\n') {
    string_free(s);
    return false;
  }
  book_t book = default_book();
  const byte_t *b = s->value;
  RETURN_FALSE(read_next_section(&b, &book.title));
  RETURN_FALSE(read_next_section(&b, &book.subtitle));
  RETURN_FALSE(read_authors(&b, book.authors));
  RETURN_FALSE(read_next_section(&b, &book.publisher));
  RETURN_FALSE(read_next_section(&b, &book.location));
  string_t *year;
  RETURN_FALSE(read_next_section(&b, &year));
  if (sscanf((char *)year->value, "%d\n", &book.year) != 1) {
    string_free(year);
    RETURN_FALSE(true);
  }
  string_free(year);
  while (*b != '\n' && *b != '\0') {
    if (*b == ',') { inc_utf8(&b); continue; }
    string_t *newstr = string_with_capacity(DEFAULT_STRING_LENGTH);
    while (*b != ',' && *b != '\n' && *b != '\0') {
      string_append_alloc(newstr, b);
      inc_utf8(&b);
    }
    stack_push(book.categories, newstr);
  }
  string_free(s);
  *bookptr = book;
  return true;
}

booknode_t *booknode_init() {
  booknode_t *node = malloc(sizeof(booknode_t));
  if (node == NULL) die("out of memory");
  return node;
}

void booknode_free(booknode_t *bn) {
  if (bn == NULL) return;
  book_free(bn->book);
  booknode_free(bn->next);
  free(bn);
}

void booknode_print_all_books(booknode_t *bn) {
  if (bn == NULL) return;
  if (bn->book.removed == false) {
    print_book(&bn->book);
    printf("\n");
  }
  booknode_print_all_books(bn->next);
}

void write_book_to_file(const book_t *book, FILE *f) {
  if (!book->removed) {
    fprint(f, book->title);
    fprintf(f, ";");
    fprint(f, book->subtitle);
    fprintf(f, ";");
    for (int auth = 0; auth < stack_size(book->authors); auth++) {
      stack_t *author = book->authors->values[auth];
      for (int name = 0; name < stack_size(author); name++) {
        fprint(f, author->values[name]);
        if (name != stack_size(author) - 1)
          fprintf(f, " ");
      }
      if (auth != stack_size(book->authors) - 1)
        fprintf(f, ",");
    }
    fprintf(f, ";");
    fprint(f, book->publisher);
    fprintf(f, ";");
    fprint(f, book->location);
    fprintf(f, ";");
    fprintf(f, "%d;", book->year);
    for (int cat = 0; cat < stack_size(book->categories); cat++) {
      fprint(f, book->categories->values[cat]);
      if (cat != stack_size(book->categories) - 1)
        fprintf(f, ",");
    }
    fprintf(f, "\n");
  }
}

void booknode_write_all_to_file(booknode_t *bn, FILE *f) {
  if (bn == NULL) return;
  write_book_to_file(&bn->book, f);
  booknode_write_all_to_file(bn->next, f);
}

bool booknode_isbook(void *bn, void *) {
  if (bn == NULL) return false;
  return ((booknode_t *)bn)->book.removed == false;
}

bool book_exists(stack_t *s) {
  return stack_exists(s, NULL, booknode_isbook);
}

booknode_t *booksll_add_book(booksll_t *booksll, book_t book) {
  if (booksll == NULL) die("booksll was null");
  booknode_t *tmphead = booksll->head;
  booksll->head = booknode_init();
  booksll->head->book = book;
  booksll->head->next = tmphead;
  return booksll->head;
}

void remove_book(booknode_t *bn) {
  if (bn == NULL) die("remove_book(): booknode was null");
  bn->book.removed = true;
}

catalogue_t *catalogue_init() {
  catalogue_t *c = calloc(1, sizeof(catalogue_t));
  if (c == NULL) die("out of memory");
  return c;
}

void catalogue_add_authors(catalogue_t *c, stack_t *authors, booknode_t *link) {
  for (int auth = 0; auth < stack_size(authors); auth++) {
    stack_t *author = authors->values[auth];
    if (stack_size(author) > 0) {
      key_t firstname = key_from_string(string_copy_alloc(author->values[0]));
      key_t lastname = key_from_string(string_copy_alloc(stack_peek(author)));
      avl_add(&c->author_first_names, firstname, link, nofree);
      avl_add(&c->author_last_names, lastname, link, nofree);
      string_t *fullname = string_with_capacity(DEFAULT_STRING_LENGTH);
      for (int i = 0; i < stack_size(author); i++) {
        string_concat_alloc(fullname, author->values[i]);
        string_append_alloc(fullname, (const byte_t *)" ");
      }
      trunc_string(fullname);
      avl_add(&c->authors, key_from_string(fullname), link, nofree);
      string_t *by_last_name = string_with_capacity(DEFAULT_STRING_LENGTH);
      string_concat_alloc(by_last_name, stack_peek(author));
      string_append_all_alloc(by_last_name, (const byte_t *)", ");
      for (int i = 0; i < stack_size(author) - 1; i++) {
        string_concat_alloc(by_last_name, author->values[i]);
        string_append_alloc(by_last_name, (const byte_t *)" ");
      }
      trunc_string(by_last_name);
      avl_add(&c->authors_by_last_name, key_from_string(by_last_name), link, nofree);
    }
  }
}

void catalogue_add_book(catalogue_t *c, book_t book) {
  if (c == NULL) die("catalogue_add_book(): catalogue was null");
  if (book.removed) return;
  booknode_t *link = booksll_add_book(&c->booklist, book);
  key_t title = key_from_string(string_copy_alloc(book.title));
  avl_add(&c->titles, title, link, nofree);
  key_t subtitle = key_from_string(string_copy_alloc(book.subtitle));
  avl_add(&c->subtitles, subtitle, link, nofree);
  catalogue_add_authors(c, book.authors, link);
  key_t publisher = key_from_string(string_copy_alloc(book.publisher));
  avl_add(&c->publishers, publisher, link, nofree);
  key_t location = key_from_string(string_copy_alloc(book.location));
  avl_add(&c->locations, location, link, nofree);
  key_t year = key_from_int(book.year);
  avl_add(&c->years, year, link, nofree);
  for (int cat = 0; cat < stack_size(book.categories); cat++) {
    string_t *catstring = book.categories->values[cat];
    key_t category = key_from_string(string_copy_alloc(catstring));
    avl_add(&c->categories, category, link, nofree);
  }
}

void catalogue_free(catalogue_t *c) {
  if (c == NULL) return;
  booknode_free(c->booklist.head);
  avl_free(c->titles);
  avl_free(c->subtitles);
  avl_free(c->authors);
  avl_free(c->authors_by_last_name);
  avl_free(c->author_last_names);
  avl_free(c->author_first_names);
  avl_free(c->publishers);
  avl_free(c->categories);
  avl_free(c->years);
  avl_free(c->locations);
  free(c);
}

void catalogue_print_all_books(catalogue_t *c) {
  if (c == NULL) die("catalogue_print_all_books(): catalogue was null");
  booknode_print_all_books(c->booklist.head);
}

int catalogue_print_walk(const key_t *k, stack_t *d, void *state) {
  if (k == NULL) die("avl walk key was null");
  if (book_exists(d)) {
    key_print(k);
    printf("\n");
  }
  return 0;
}

void catalogue_print_all_titles(catalogue_t *c) {
  if (c == NULL) die("catalogue_print_all_titles(): catalogue was null");
  avl_walk(c->titles, catalogue_print_walk, NULL);
}

void catalogue_print_all_subtitles(catalogue_t *c) {
  if (c == NULL) die("catalogue_print_all_subtitles(): catalogue was null");
  avl_walk(c->subtitles, catalogue_print_walk, NULL);
}

void catalogue_print_all_authors(catalogue_t *c) {
  if (c == NULL) die("catalogue_print_all_authors(): catalogue was null");
  avl_walk(c->authors, catalogue_print_walk, NULL);
}

void catalogue_print_all_authors_by_last_name(catalogue_t *c) {
  if (c == NULL) die("catalogue_print_all_authors_by_last_name(): catalogue was null");
  avl_walk(c->authors_by_last_name, catalogue_print_walk, NULL);
}

void catalogue_print_all_author_last_names(catalogue_t *c) {
  if (c == NULL) die("catalogue_print_all_author_last_names(): catalogue was null");
  avl_walk(c->author_last_names, catalogue_print_walk, NULL);
}

void catalogue_print_all_author_first_names(catalogue_t *c) {
  if (c == NULL) die("catalogue_print_all_author_first_names(): catalogue was null");
  avl_walk(c->author_first_names, catalogue_print_walk, NULL);
}

void catalogue_print_all_publishers(catalogue_t *c) {
  if (c == NULL) die("catalogue_print_all_publishers(): catalogue was null");
  avl_walk(c->publishers, catalogue_print_walk, NULL);
}

void catalogue_print_all_years(catalogue_t *c) {
  if (c == NULL) die("catalogue_print_all_years(): catalogue was null");
  avl_walk(c->years, catalogue_print_walk, NULL);
}

void catalogue_print_all_categories(catalogue_t *c) {
  if (c == NULL) die("catalogue_print_all_categories(): catalogue was null");
  avl_walk(c->categories, catalogue_print_walk, NULL);
}

void catalogue_print_all_locations(catalogue_t *c) {
  if (c == NULL) die("catalogue_print_all_locations(): catalogue was null");
  avl_walk(c->locations, catalogue_print_walk, NULL);
}

void catalogue_write_to_file(catalogue_t *c, string_t *filename) {
  if (c == NULL) die("catalogue_write_to_file(): catalogue was null");
  if (filename == NULL) {
    printf("Invalid filename, try again\n");
    return;
  }
  if (filename->value == NULL) {
    printf("Invalid filename, try again\n");
    return;
  }
  FILE *f = fopen((char *)filename->value, "w");
  if (f == NULL) {
    printf("Invalid filename, try again\n");
    return;
  }
  booknode_write_all_to_file(c->booklist.head, f);
  fclose(f);
}

int catalogue_read_from_file(catalogue_t *c, string_t *filename) {
  if (c == NULL) die("catalogue_read_from_file(): catalogue was null");
  if (filename == NULL) {
    printf("Invalid filename, try again\n");
    return 1;
  }
  if (filename->value == NULL) {
    printf("Invalid filename, try again\n");
    return 1;
  }
  FILE *f = fopen((char *)filename->value, "r");
  if (f == NULL) {
    printf("Invalid filename, try again\n");
    return 1;
  }
  book_t book;
  while (book_read_from_file(f, &book)) {

    catalogue_add_book(c, book);
  }
  fclose(f);
  return 0;
}

// implement searching by year
void catalogue_search(catalogue_t *c) {
  printf("Search area: ");
  string_t *area = file_read_line_alloc(stdin);
  trunc_string(area);
  if (area->len == 0) return;
  const char *buf = (char *)area->value;
  if (strcmp(buf, "h") == 0 || strcmp(buf, "help") == 0) {
    print_search_help();
  } else if (strcmp(buf, "t") == 0 || strcmp(buf, "title") == 0) {
    string_free(area);
    printf("Search titles: ");
    catalogue_search_avl(c->titles);
  } else if (strcmp(buf, "st") == 0 || strcmp(buf, "subtitle") == 0) {
    string_free(area);
    printf("Search subtitles: ");
    catalogue_search_avl(c->subtitles);
  } else if (strcmp(buf, "a") == 0 || strcmp(buf, "author") == 0) {
    string_free(area);
    printf("Search authors: ");
    catalogue_search_avl(c->authors);
  } else if (strcmp(buf, "l") == 0 || strcmp(buf, "lastname") == 0) {
    string_free(area);
    printf("Search authors: ");
    catalogue_search_avl(c->authors_by_last_name);
  } else if (strcmp(buf, "al") == 0 || strcmp(buf, "authorlast") == 0) {
    string_free(area);
    printf("Search author last names: ");
    catalogue_search_avl(c->author_last_names);
  } else if (strcmp(buf, "af") == 0 || strcmp(buf, "authorfirst") == 0) {
    string_free(area);
    printf("Search author first names: ");
    catalogue_search_avl(c->author_first_names);
  } else if (strcmp(buf, "p") == 0 || strcmp(buf, "pub") == 0) {
    string_free(area);
    printf("Search publishers: ");
    catalogue_search_avl(c->publishers);
  } else if (strcmp(buf, "y") == 0 || strcmp(buf, "year") == 0) {
    printf("Year not implemented yet\n");
  } else if (strcmp(buf, "c") == 0 || strcmp(buf, "cat") == 0) {
    string_free(area);
    printf("Search categories: ");
    catalogue_search_avl(c->categories);
  } else if (strcmp(buf, "lc") == 0 || strcmp(buf, "location") == 0) {
    string_free(area);
    printf("Search locations: ");
    catalogue_search_avl(c->locations);
  } else {
    printf("\nUnknown search area\n");
  }
}

void catalogue_search_avl(const avl_t *avl) {
  string_t *s = file_read_line_alloc(stdin);
  trunc_string(s);
  key_t key = key_from_string(s);
  stack_t *stack = avl_get(avl, &key);
  if (stack == NULL) return;
  for (size_t b = 0; b < stack_size(stack); b++) {
    booknode_t *bn = stack->values[b];
    printf("\n");
    print_book(&bn->book);
  }
}

void print_search_help() {
  printf("%sCatalogue Search Areas:%s\n", BWHT, CRESET);
  printf(" h,  help         print this help message\n");
  printf(" t,  title        search for a book title\n");
  printf(" st, subtitle     search for a book subtitle\n");
  printf(" a,  author       search for author by full name\n");
  printf(" l,  lastname     search for author by full name, starting\n");
  printf("                   with last name (e.g. 'Hinton, Matthew')\n");
  printf(" al, authorlast   search by author last name\n");
  printf(" af, authorfirst  search by author first name\n");
  printf(" p,  pub          search for a publisher\n");
  printf(" y,  year         search by publication year\n");
  printf(" c,  cat          search for a category or list of categories\n");
  printf(" lc, location     search by location\n");
}

// write help message
void print_help() {
  printf("help not implemented yet\n");
  printf("\n");
}
