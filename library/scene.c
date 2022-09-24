#include "scene.h"
#include "body.h"
#include "color.h"
#include "force.h"
#include "info.h"
#include "polygon.h"
#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>

const size_t INITIAL_BODIES_GUESS = 15;
const size_t INITIAL_FORCES_GUESS = 30;
const size_t GRAV = 3; // last 3 spots reserved for gravity
const size_t FAN = 4; // 4 spots reserved for fan (before gravity)

const size_t PLY1 = 1;
const size_t PLY2 = 2;
const size_t BLOCK_BODY = 3;

typedef struct scene {
  list_t *bodies;
  list_t *hidden_bodies;
  list_t *force;
  size_t num_bodies;
  size_t counter;
  bool win;
  bool lose;
} scene_t;

/**
 * Allocates memory for an empty scene.
 * Makes a reasonable guess of the number of bodies to allocate space for.
 * Asserts that the required memory is successfully allocated.
 *
 * @return the new scene
 */
scene_t *scene_init(void) {
  scene_t *s = malloc(sizeof(scene_t));
  assert(s != NULL);
  list_t *scene_bodies =
      list_init(INITIAL_BODIES_GUESS, (free_func_t)body_free);
  list_t *hidden_bodies =
      list_init(INITIAL_BODIES_GUESS, (free_func_t)body_free);
  list_t *scene_forces =
      list_init(INITIAL_FORCES_GUESS, (free_func_t)force_free);
  s->bodies = scene_bodies;
  s->hidden_bodies = hidden_bodies;
  s->force = scene_forces;
  s->num_bodies = 0;
  s->lose = false;
  s->win = false;
  s->counter = 0;
  return s;
}

/**
 * Releases memory allocated for a given scene and all its bodies.
 *
 * @param scene a pointer to a scene returned from scene_init()
 */
void scene_free(scene_t *scene) {
  list_free(scene->bodies);
  list_free(scene->hidden_bodies);
  scene_free_forces(scene);
  free(scene);
}

size_t scene_bodies(scene_t *scene) { return scene->num_bodies; }

size_t scene_counter(scene_t *scene) { return scene->counter; }

body_t *scene_get_body(scene_t *scene, size_t index) {
  assert(index < scene->num_bodies);
  return (body_t *)list_get(scene->bodies, index);
}

void scene_add_body(scene_t *scene, body_t *body) {
  list_add(scene->bodies, body);
  scene->num_bodies++;
}

void scene_add_hidden_body(scene_t *scene, body_t *body) {
  list_add(scene->hidden_bodies, body);
}

void scene_remove_body(scene_t *scene, size_t index) {
  assert(index < scene->num_bodies);
  body_remove(list_get(scene->bodies, index));
}

/**
 * Executes a tick of a given scene over a small time interval.
 * This requires executing all the force creators
 * and then ticking each body (see body_tick()).
 *
 * @param scene a pointer to a scene returned from scene_init()
 * @param dt the time elapsed since the last tick, in seconds
 */
void scene_tick(scene_t *scene, double dt) {
  if (scene->num_bodies > BLOCK_BODY && (body_get_lose(scene_get_body(scene, PLY1)) || body_get_lose(scene_get_body(scene, PLY2)))) {
    scene->lose = true;
  }
  if (scene->num_bodies > BLOCK_BODY && (body_get_win(scene_get_body(scene, PLY1)) || body_get_win(scene_get_body(scene, PLY2)))) {
    scene->win = true;
  }
  size_t end = list_size(scene->force);
  if (scene->num_bodies > BLOCK_BODY && (body_get_fan(scene_get_body(scene, PLY1)) || body_get_fan(scene_get_body(scene, PLY2)) || body_get_fan(scene_get_body(scene, BLOCK_BODY)))) {
    end -= FAN + GRAV;
  }

  for (int i = end + FAN; i < list_size(scene->force); i++) {
    get_force_creator((force_t *)list_get(scene->force, i))(
        get_aux(((force_t *)list_get(scene->force, i))));
  }
  for (int i = end - 1; i >= 0; i--) {
    get_force_creator((force_t *)list_get(scene->force, i))(
        get_aux(((force_t *)list_get(scene->force, i))));
  }

  for (size_t i = scene->num_bodies; i > 0; i--) {
    if (body_is_removed(scene_get_body(scene, 0)) &&
        body_get_info_freer(scene_get_body(scene, 0)) != NULL &&
        get_typ((info_t *)body_get_info(scene_get_body(scene, 0))) == 0) {
      exit(0);
    }
    body_t *curr = scene_get_body(scene, i - 1);
    if (body_is_removed(curr)) {
      for (size_t j = list_size(scene->force); j > 0; j--) {
        list_t *bods =
            get_relevant_bodies((force_t *)list_get(scene->force, j - 1));
        for (size_t k = 0; k < list_size(bods); k++) {
          if (list_get(bods, k) == curr) {
            force_t *rmv = list_get(scene->force, j - 1);
            list_remove(scene->force, j - 1);
            force_free(rmv);
            break;
          }
        }
      }
      body_t *rem = list_get(scene->bodies, i - 1);
      list_remove(scene->bodies, i - 1);
      body_free(rem);
      scene->num_bodies--;
      scene->counter++;
    } else {
      body_tick(list_get(scene->bodies, i - 1), dt);
    }
  }
}

void scene_add_force_creator(scene_t *scene, force_creator_t forcer, void *aux,
                             free_func_t freer) {
  force_t *f = force_init(forcer, aux, freer);
  list_add(scene->force, f);
}

void scene_free_forces(scene_t *scene) { list_free(scene->force); }

void scene_add_bodies_force_creator(scene_t *scene, force_creator_t forcer,
                                    void *aux, list_t *bodies,
                                    free_func_t freer) {
  force_t *f = force_init_with_bodies(forcer, aux, bodies, freer);
  list_add(scene->force, f);
}

bool scene_get_lose(scene_t *scene) {return scene->lose; }
bool scene_get_win(scene_t *scene) {return scene->win; }
