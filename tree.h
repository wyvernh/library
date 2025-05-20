#ifndef TREE_H_
#define TREE_H_
#include "library.h"
#include <stdint.h>

typedef int (*avl_walkfunc_t)(const key_t *k, stack_t *d, void *state);

key_t key_from_string(string_t *s);

key_t key_from_int(int i);

void key_free(key_t k);

void key_fprint(FILE *f, const key_t *k);

void key_print(const key_t *k);

bool key_is_void(const key_t *key);

avl_t *avl_alloc();

void avl_free(avl_t *avl);

uintptr_t avl_size(avl_t *avl);

void avl_print(const avl_t *avl);

bool avl_contains(const avl_t *avl, const key_t *key);

stack_t *avl_get(const avl_t *avl, const key_t *key);

uintptr_t avl_height(avl_t *avl);

void avl_update_height(avl_t *avl);

avl_t *avl_rotate_right(avl_t *root);

avl_t *avl_rotate_left(avl_t *root);

avl_t *avl_rotate_left_node(avl_t *root);

avl_t *avl_rotate_right_node(avl_t *root);

int avl_left_minus_right(avl_t *avl);

avl_t *avl_rotate_correctly(avl_t *root);

avl_t *avl_updated_node(avl_t *root);

void avl_add(avl_t **avl, key_t k, void *v, void(*freefunc)(void *));

avl_t *drop_min_from_left(avl_t **root, avl_t *left);

avl_t *avl_drop_min(avl_t **root);

avl_t *avl_combine_branches(avl_t *l, avl_t *r);

avl_t *avl_remove_node(avl_t **root, const key_t *key);

void *avl_remove(avl_t **avl, const key_t *key);

int avl_walk(avl_t *avl, avl_walkfunc_t walkfunc, void *state);

int avl_print_list_walkfunc(const key_t *key, stack_t *data, void *file);

#endif // TREE_H_
