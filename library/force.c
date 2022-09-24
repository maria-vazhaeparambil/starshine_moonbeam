#include "force.h"
#include "list.h"
#include "scene.h"
#include <stdlib.h>

typedef struct force {
  force_creator_t forcer;
  void *aux;
  free_func_t freer;
  list_t *relevant_bodies;
} force_t;

force_t *force_init(force_creator_t forcer, void *aux, free_func_t freer) {
  force_t *f = malloc(sizeof(force_t));
  f->forcer = forcer;
  f->aux = aux;
  f->freer = freer;
  f->relevant_bodies = NULL;
  return f;
}

force_t *force_init_with_bodies(force_creator_t forcer, void *aux,
                                list_t *relevant_bodies, free_func_t freer) {
  force_t *f = malloc(sizeof(force_t));
  f->forcer = forcer;
  f->aux = aux;
  f->freer = freer;
  f->relevant_bodies = relevant_bodies;
  return f;
}

void force_free(force_t *force) {
  if (force->freer != NULL) {
    force->freer(force->aux);
  }
  if (force->relevant_bodies != NULL) {
    list_free(force->relevant_bodies);
  }
  free(force);
}

force_creator_t get_force_creator(force_t *force) { return force->forcer; }

void *get_aux(force_t *force) { return force->aux; }

free_func_t get_freer(force_t *force) { return force->freer; }

list_t *get_relevant_bodies(force_t *force) { return force->relevant_bodies; }
