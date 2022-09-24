#include "list.h"
#include "polygon.h"
#include "vector.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct list {
  void **items;
  size_t size;
  size_t capacity;
  free_func_t freer;
} list_t;

list_t *list_init(size_t initial_size, free_func_t freer) {
  list_t *body = malloc(sizeof(list_t));
  assert(body != NULL);
  body->items = malloc(initial_size * sizeof(void *));
  assert(body->items != NULL);
  body->size = 0;
  body->capacity = initial_size;
  body->freer = freer;
  return body;
}

void list_free(list_t *list) {
  if (list->freer != NULL) {
    for (size_t i = 0; i < list_size(list); i++) {
      free_func_t f = list->freer;
      f(list->items[i]);
    }
  }
  free(list->items);
  free(list);
}

size_t list_size(list_t *list) { return list->size; }

void *list_get(list_t *list, size_t index) {
  assert(index < list_size(list));
  return list->items[index];
}

void *list_remove(list_t *list, size_t index) {
  assert(list_size(list) > 0);
  void *store = list->items[index];
  if (index != list_size(list) - 1) {
    for (size_t i = index + 1; i < list_size(list); i++) {
      list->items[i - 1] = list->items[i];
    }
    list->items[list_size(list) - 1] = store;
  }
  list->size--;
  return store;
}

void list_resize(list_t *list) {
  if (list->capacity == 0) {
    free(list->items);
    list->capacity = 1;
    void **placeholder = malloc(sizeof(void *));
    list->items = placeholder;
    return;
  }
  void **placeholder = malloc(list->capacity * sizeof(void *) * 2);
  for (int i = 0; i < list->capacity; i++) {
    placeholder[i] = list_get(list, i);
  }
  list->capacity = list->capacity * 2;
  free(list->items);
  list->items = placeholder;
}

void list_add(list_t *list, void *value) {
  assert(value != NULL);
  if (list_size(list) == list->capacity) {
    list_resize(list);
  }
  list->items[list_size(list)] = value;
  list->size++;
}
