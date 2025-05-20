#include <stdio.h>
#include <stdlib.h>

void die(void *message) {
  fprintf(stderr, "ERROR: %s\n", (char *)message);
  exit(1);
}

void *replace(void *ptr, void *data) {
  if (ptr == NULL)
    die("replace(): ptr was NULL");
  void **p = ptr;
  ptr = *p;
  *p = data;
  return ptr;
}

void *take(void *ptr) {
  return replace(ptr, NULL);
}

void swap(void *ptr1, void *ptr2) {
  if (ptr1 == NULL || ptr2 == NULL)
    die("swap(): ptr was NULL");
  void *p = *(void **)ptr1;
  *(void **)ptr1 = *(void **)ptr2;
  *(void **)ptr2 = p;
}

void *expect(void *ptr, void *message) {
  if (ptr == NULL) die(message);
  return *(void **)ptr;
}

void *unwrap(void *ptr) {
  return expect(ptr, "unwrap was called on NULL");
}

void nofree(void *) { }
