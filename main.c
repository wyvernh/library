#include <stdio.h>
#include <string.h>
#include "macros.h"
#include "library.h"

bool add_book(library_t *library, FILE *f) {
  if (library == NULL) die("add_book(): library was null");
  book_t book;
  if (read_book(&book) == false)
    return false;
  write_book_to_file(&book, f);
  fflush(f);
  catalogue_add_book(library->catalogue, book);
  return true;
}

void add_books(library_t *library, FILE *f) {
  while (true) {
    printf("\n");
    if (add_book(library, f) == false) {
      printf("Are you sure you want to exit? [y/n]: ");
      string_t *answer = file_read_line_alloc(stdin);
      trunc_string(answer);
      bool y   = strcmp((char *)answer->value, "y") == 0;
      bool yes = strcmp((char *)answer->value, "yes") == 0;
      string_free(answer);
      if (y || yes) return;
    }
  }
}

void print_catalogue(const library_t *library) {
  printf("%sBooks:%s\n", BWHT, CRESET);
  catalogue_print_all_books(library->catalogue);
  printf("\n%sTitles:%s\n", BWHT, CRESET);
  catalogue_print_all_titles(library->catalogue);
  printf("\n%sSubtitles:%s\n", BWHT, CRESET);
  catalogue_print_all_subtitles(library->catalogue);
  printf("\n%sAuthors:%s\n", BWHT, CRESET);
  catalogue_print_all_authors(library->catalogue);
  printf("\n%sAuthors by Last Name:%s\n", BWHT, CRESET);
  catalogue_print_all_authors_by_last_name(library->catalogue);
  printf("\n%sAuthor Last Names:%s\n", BWHT, CRESET);
  catalogue_print_all_author_last_names(library->catalogue);
  printf("\n%sAuthor First Names:%s\n", BWHT, CRESET);
  catalogue_print_all_author_first_names(library->catalogue);
  printf("\n%sPublishers:%s\n", BWHT, CRESET);
  catalogue_print_all_publishers(library->catalogue);
  printf("\n%sYears:%s\n", BWHT, CRESET);
  catalogue_print_all_years(library->catalogue);
  printf("\n%sCategories:%s\n", BWHT, CRESET);
  catalogue_print_all_categories(library->catalogue);
}

bool command(const string_t *cmd, library_t *library, FILE *f) {
  if (cmd->len == 0) return false;
  const char *buf = (char *)cmd->value;
  if (strcmp(buf, "q") == 0 || strcmp(buf, "quit") == 0) {
    return true;
  } else if (strcmp(buf, "h") == 0 || strcmp(buf, "help") == 0) {
    print_help();
  } else if (strcmp(buf, "b") == 0 || strcmp(buf, "books") == 0) {
    printf("%sBooks:%s\n", BWHT, CRESET);
    catalogue_print_all_books(library->catalogue);
  } else if (strcmp(buf, "t") == 0 || strcmp(buf, "titles") == 0) {
    printf("%sTitles:%s\n", BWHT, CRESET);
    catalogue_print_all_titles(library->catalogue);
  } else if (strcmp(buf, "st") == 0 || strcmp(buf, "subtitles") == 0) {
    printf("%sSubtitles:%s\n", BWHT, CRESET);
    catalogue_print_all_subtitles(library->catalogue);
  } else if (strcmp(buf, "a") == 0 || strcmp(buf, "authors") == 0) {
    printf("%sAuthors:%s\n", BWHT, CRESET);
    catalogue_print_all_authors(library->catalogue);
  } else if (strcmp(buf, "l") == 0 || strcmp(buf, "lastname") == 0) {
    printf("%sAuthors by Last Name:%s\n", BWHT, CRESET);
    catalogue_print_all_authors_by_last_name(library->catalogue);
  } else if (strcmp(buf, "al") == 0 || strcmp(buf, "authorlast") == 0) {
    printf("%sAuthor Last Names:%s\n", BWHT, CRESET);
    catalogue_print_all_author_last_names(library->catalogue);
  } else if (strcmp(buf, "af") == 0 || strcmp(buf, "authorfirst") == 0) {
    printf("%sAuthor First Names:%s\n", BWHT, CRESET);
    catalogue_print_all_author_first_names(library->catalogue);
  } else if (strcmp(buf, "p") == 0 || strcmp(buf, "pub") == 0) {
    printf("%sPublishers:%s\n", BWHT, CRESET);
    catalogue_print_all_publishers(library->catalogue);
  } else if (strcmp(buf, "y") == 0 || strcmp(buf, "years") == 0) {
    printf("%sYears:%s\n", BWHT, CRESET);
    catalogue_print_all_years(library->catalogue);
  } else if (strcmp(buf, "c") == 0 || strcmp(buf, "cat") == 0) {
    printf("%sCategories:%s\n", BWHT, CRESET);
    catalogue_print_all_categories(library->catalogue);
  } else if (strcmp(buf, "add") == 0) {
    add_book(library, f);
  } else if (strcmp(buf, "addbooks") == 0 || strcmp(buf, "add books") == 0) {
    add_books(library, f);
  } else if (strcmp(buf, "s") == 0 || strcmp(buf, "search") == 0) {
    catalogue_search(library->catalogue);
  } else {
    printf("\nUnknown command\n");
  }
  return false;
}

int main(int argc, char **argv) {
  if (argc > 2) {
    printf("Too many arguments\n");
    return 1;
  }

  library_t library = { LIB_OK, NULL };
  library.catalogue = catalogue_init();

  if (argc == 1) {
    printf("File to store library catalogue in: ");
    string_t *filename = file_read_line_alloc(stdin);
    trunc_string(filename);
    FILE *f = fopen((char *)filename->value, "w");
    string_free(filename);
    if (f == NULL) {
      printf("could not open file\n");
      return 1;
    }
    add_books(&library, f);
    fclose(f);
    catalogue_free(library.catalogue);
    return 0;
  }

  string_t *filename = string_from_alloc(argv[1]);
  RET_IF(catalogue_read_from_file(library.catalogue, filename));
  string_free(filename);

  FILE *f = fopen(argv[1], "w");
  if (f == NULL) {
    printf("could not open file for writing\n");
    return 1;
  }
  booknode_write_all_to_file(library.catalogue->booklist.head, f);
  fflush(f);

  print_catalogue(&library);

  // command loop
  while (true) {
    printf("\n>>> ");
    string_t *cmd = file_read_line_alloc(stdin);
    trunc_string(cmd);
    bool done = command(cmd, &library, f);
    string_free(cmd);
    if (done) break;
  }

  fclose(f);
  catalogue_free(library.catalogue);

  return 0;
}
