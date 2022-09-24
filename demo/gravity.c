#include "list.h"
#include "polygon.h"
#include "sdl_wrapper.h"
#include "star_body.h"
#include "state.h"
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

const vector_t WINDOW = (vector_t){.x = 1000, .y = 500};
const vector_t CENTER = (vector_t){.x = 500, .y = 250};

const size_t NUM_POINTS = 3;
const size_t POINTS_MAX = 16;
const size_t POINTS_MIN = 2;

const double GRAVITY = -9.81 / 2.0;

const double ROTATION_ANGLE = M_PI / 180.0;

const int SIZE = 10;

typedef struct state {
  size_t points;
  size_t start;
  size_t end;
  list_t *list;
} state_t;

bool check_bounce(list_t *points, vector_t velocity) {
  for (size_t i = 0; i < list_size(points); i++) {
    if (((vector_t *)list_get(points, i))->y <= 0) {
      vector_t translate_pt = (vector_t){
          .x = 0.0, .y = (-1 * ((vector_t *)list_get(points, i))->y)};
      polygon_translate(points, translate_pt);
      return true;
    }
  }
  return false;
}

bool check_deletions(list_t *points) {
  for (size_t i = 0; i < list_size(points); i++) {
    if (((vector_t *)list_get(points, i))->x < WINDOW.x) {
      return false;
    }
  }
  return true;
}

state_t *emscripten_init() {
  vector_t min = (vector_t){.x = 0, .y = 0};
  vector_t max = WINDOW;
  sdl_init(min, max);

  state_t *s = malloc(sizeof(state_t));
  assert(s != NULL);
  s->list = list_init(SIZE, free);
  s->start = 0;
  s->end = 1;
  s->points = NUM_POINTS;

  generate_star(s->list, NUM_POINTS, WINDOW);

  return s;
}

void emscripten_main(state_t *s) {
  sdl_clear();
  list_t *arr = s->list;
  double t = time_since_last_tick();
  for (int i = s->start; i < s->end; i++) {
    polygon_t *curr = ((polygon_t *)list_get(arr, i));
    polygon_rotate(curr->points, ROTATION_ANGLE, curr->centroid);
    polygon_translate(curr->points, curr->velocity);
    double yvelocity = curr->velocity.y;
    curr->velocity.y = yvelocity + GRAVITY * t;
    curr->centroid = vec_add(curr->centroid, curr->velocity);

    if (check_bounce(curr->points, curr->velocity)) {
      curr->velocity.y *= -1 * curr->elas;
    }
    if (generate_new_star(arr, i, yvelocity, curr->velocity.y, SIZE)) {
      generate_star(arr, NUM_POINTS + list_size(arr), WINDOW);
      s->end = s->end + 1;
    }
    if (check_deletions(curr->points)) {
      s->start = s->start + 1;
    }

    polygon_rotate(curr->points, ROTATION_ANGLE, curr->centroid);
    polygon_translate(curr->points, curr->velocity);
    double t = time_since_last_tick();
    curr->velocity.y = curr->velocity.y + GRAVITY * t;
    curr->centroid = vec_add(curr->centroid, curr->velocity);
    rgb_color_t col = {curr->red, curr->green, curr->blue};
    sdl_draw_polygon(curr->points, col);
  }
  sdl_show();
}

void emscripten_free(state_t *s) {
  list_t *arr = s->list;
  for (int i = 0; i < s->end; i++) {
    polygon_t *curr = ((polygon_t *)list_get(arr, i));
    list_free(curr->points);
    free(curr);
  }
  list_free(arr);
  free(arr);
  free(s);
}
