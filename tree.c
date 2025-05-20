#include "tree.h"
#include "macros.h"
#include "library.h"
#include <stdlib.h>
#include <string.h>

key_t key_from_string(string_t *s) {
  key_t k = { KEY_STRING, .key = s };
  return k;
}

key_t key_from_int(int i) {
  key_t k = { KEY_INT, .ikey = i };
  return k;
}

void key_free(key_t k) {
  if (k.type == KEY_STRING)
    string_free(k.key);
}

void key_fprint(FILE *f, const key_t *k) {
  if (k == NULL) die("key pointer was null");
  if (k->type == KEY_STRING)
    fprint(f, k->key);
  else
    fprintf(f, "%d", k->ikey);
}

void key_print(const key_t *k) {
  key_fprint(stdout, k);
}

bool key_is_void(const key_t *key) {
  if (key->type == KEY_INT) return false;
  if (string_length(key->key) == 0) return true;
  return false;
}

int key_comp(const key_t *k1, const key_t *k2) {
  if (k1 == NULL || k2 == NULL) die ("key pointer was null");
  if (k1->type != k2->type) die("key type error");
  if (k1->type == KEY_STRING)
    return string_comp(k1->key, k2->key);
  return k1->ikey - k2->ikey;
}

avl_t *avl_alloc() {
  avl_t *avl = calloc(1, sizeof(avl_t));
  if (avl == NULL) die("out of memory");
  avl->freefunc = nofree;
  return avl;
}

void avl_free(avl_t *avl) {
  if (avl == NULL) return;
  avl_free(avl->left);
  avl_free(avl->right);
  key_free(avl->key);
  stack_free(avl->data, avl->freefunc);
  free(avl);
}

uintptr_t avl_size(avl_t *avl) {
  if (avl == NULL) return 0;
  return avl_size(avl->left) + 1 + avl_size(avl->right);
}

void avl_print(const avl_t *avl) {
  if (avl == NULL) return;
  printf("(");
  avl_print(avl->left);
  if (avl->left) printf(" ");
  key_print(&avl->key);
  if (avl->right) printf(" ");
  avl_print(avl->right);
  printf(")");
}

bool avl_contains(const avl_t *avl, const key_t *key) {
  if (avl == NULL) return false;
  int comp = key_comp(key, &avl->key);
  if (comp < 0) return avl_contains(avl->left, key);
  if (comp > 0) return avl_contains(avl->right, key);
  return true;
}

stack_t *avl_get(const avl_t *avl, const key_t *key) {
  if (avl == NULL) return NULL;
  int comp = key_comp(key, &avl->key);
  if (comp < 0) return avl_get(avl->left, key);
  if (comp > 0) return avl_get(avl->right, key);
  return avl->data;
}

uintptr_t avl_height(avl_t *avl) {
  if (avl == NULL) return 0;
  return avl->height;
}

void avl_update_height(avl_t *avl) {
  if (avl == NULL) return;
  avl->height = max(avl_height(avl->left), avl_height(avl->right)) + 1;
}

avl_t *avl_rotate_right(avl_t *root) {
  if (root == NULL) die("avl_rotate_right(): invalid node");
  if (root->left == NULL) die("avl_rotate_right(): invalid rotate");
  avl_t *newroot = root->left;
  root->left = take(&newroot->right);
  avl_update_height(root);
  newroot->right = root;
  avl_update_height(newroot);
  return newroot;
}

avl_t *avl_rotate_left(avl_t *root) {
  if (root == NULL) die("avl_rotate_left(): invalid node");
  if (root->right == NULL) die("avl_rotate_left(): invalid rotate");
  avl_t *newroot = root->right;
  root->right = take(&newroot->left);
  avl_update_height(root);
  newroot->left = root;
  avl_update_height(newroot);
  return newroot;
}

avl_t *avl_rotate_left_node(avl_t *root) {
  if (root == NULL) die("avl_rotate_left_node(): invalid node");
  if (root->left == NULL) die("avl_rotate_right_node(): invalid rotate");
  avl_t *left = root->left;
  if (avl_height(left->left) < avl_height(left->right)) {
    root->left = avl_rotate_left(left);
    avl_update_height(root);
  }
  return avl_rotate_right(root);
}

avl_t *avl_rotate_right_node(avl_t *root) {
  if (root == NULL) die("avl_rotate_left_node(): invalid node");
  if (root->right == NULL) die("avl_rotate_right_node(): invalid rotate");
  avl_t *right = root->right;
  if (avl_height(right->left) > avl_height(right->right)) {
    root->right = avl_rotate_right(right);
    avl_update_height(root);
  }
  return avl_rotate_left(root);
}

int avl_left_minus_right(avl_t *avl) {
  return avl_height(avl->left) - avl_height(avl->right);
}

avl_t *avl_rotate_correctly(avl_t *root) {
  int diff = avl_left_minus_right(root);
  if (-1 <= diff && diff <= 1) return root;
  if (diff ==  2) return avl_rotate_left_node(root);
  if (diff == -2) return avl_rotate_right_node(root);
  die("avl_rotate_correctly(): invalid tree balance");
  return NULL;
}

void avl_update_node(avl_t **root) {
  if (root == NULL) die("avl_update_node(): root is NULL");
  *root = avl_rotate_correctly(*root);
  avl_update_height(*root);
}

void avl_add(avl_t **root, key_t key, void *v, void(*freefunc)(void *)) {
  if (root == NULL) die("avl root was null");
  if (key_is_void(&key)) {
    key_free(key);
    freefunc(v);
    return;
  }
  if (*root == NULL) {
    *root = avl_alloc();
    (*root)->key = key;
    (*root)->data = stack_init(1);
    stack_push((*root)->data, v);
    (*root)->height = 1;
    (*root)->freefunc = freefunc;
    return;
  }
  avl_t *avl = *root;
  int diff = key_comp(&key, &avl->key);
  if (diff == 0) {
    key_free(avl->key);
    avl->key = key;
    stack_push(avl->data, v);
    return;
  }
  if (diff > 0)
    avl_add(&avl->right, key, v, freefunc);
  else
    avl_add(&avl->left, key, v, freefunc);
  avl_update_node(root);
  return;
}

avl_t *drop_min_from_left(avl_t **root, avl_t *left) {
  (*root)->left = avl_drop_min(&left);
  avl_t *oldroot = *root;
  *root = left;
  avl_update_node(&oldroot);
  return oldroot;
}

avl_t *avl_drop_min(avl_t **root) {
  avl_t *left = take(&(*root)->left);
  if (left == NULL)
    return take(&(*root)->right);
  return drop_min_from_left(root, left);
}

avl_t *avl_combine_branches(avl_t *l, avl_t *r) {
  if (l == NULL) return r;
  if (r == NULL) return l;
  avl_t *trunc_right = avl_drop_min(&r);
  r->left = l;
  r->right = trunc_right;
  avl_update_node(&r);
  return r;
}

avl_t *avl_remove_node(avl_t **root, const key_t *key) {
  if (root == NULL) die("avl root was null");
  if (*root == NULL) return NULL;
  avl_t *avl = *root;
  int diff = key_comp(key, &avl->key);
  if (diff == 0) {
    *root = avl_combine_branches(take(&avl->left), take(&avl->right));
    return avl;
  }
  if (diff > 0)
    avl = avl_remove_node(&avl->right, key);
  else
    avl = avl_remove_node(&avl->left, key);
  avl_update_node(root);
  return avl;
}

void *avl_remove(avl_t **avl, const key_t *key) {
  if (avl == NULL) die("avl root was null");
  stack_t *target = avl_get(*avl, key);
  if (target == NULL) return NULL;
  if (target->size > 1) {
    return stack_pop(target);
  }
  avl_t *node = avl_remove_node(avl, key);
  if (node == NULL) return NULL;
  void *data = stack_pop(node->data);
  avl_free(node);
  return data;
}

int avl_walk(avl_t *avl, avl_walkfunc_t walkfunc, void *state) {
  if (avl == NULL) return 0;
  RET_IF(avl_walk(avl->left, walkfunc, state));
  RET_IF(walkfunc(&avl->key, avl->data, state));
  return avl_walk(avl->right, walkfunc, state);
}

int avl_print_list_walkfunc(const key_t *key, stack_t *data, void *file) {
  FILE *f;
  if (file != NULL) f = file;
  else              f = stdout;
  fprintf(f, " '");
  key_fprint(f, key);
  fprintf(f, "'");
  return 0;
}
