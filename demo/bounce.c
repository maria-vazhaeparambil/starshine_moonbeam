#include "color.h"
#include "list.h"
#include "polygon.h"
#include "sdl_wrapper.h"
#include "star_body.h"
#include "state.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

const vector_t WINDOW = (vector_t){.x = 1000, .y = 500};
const vector_t CENTER = (vector_t){.x = 500, .y = 250};
const vector_t VEL = (vector_t){.x = 1, .y = 1};

const size_t NUM_POINTS_BOUNCE = 5;
const size_t STAR_RADIUS_BOUNCE = 50;
const double angle = (2 * M_PI / NUM_POINTS_BOUNCE);

const float PURPLE_R = 0.75;
const float PURPLE_B = 1;
const float PURPLE_G = 0.0;
const double ROTATION_ANGLE = 0.75;
const int SIZE = 1;

typedef struct state {
  size_t points;
  size_t start;
  size_t end;
  list_t *list;
} state_t;

vector_t check_bounce(list_t *points, vector_t velocity) {
  for (size_t i = 0; i < list_size(points); i++) {
    if (((vector_t *)list_get(points, i))->x >= 1000 ||
        ((vector_t *)list_get(points, i))->x <= 0) {
      return (vector_t){.x = velocity.x * -1, .y = velocity.y};
    } else if (((vector_t *)list_get(points, i))->y >= 500 ||
               ((vector_t *)list_get(points, i))->y <= 0) {
      return (vector_t){.x = velocity.x, .y = velocity.y * -1};
    }
  }
  return velocity;
}

state_t *emscripten_init() {
  vector_t min = (vector_t){.x = 0, .y = 0};
  vector_t max = WINDOW;
  sdl_init(min, max);

  state_t *state = malloc(sizeof(state_t));
  assert(state != NULL);
  state->list = list_init(SIZE, free);
  state->start = 0;
  state->end = 1;
  state->points = NUM_POINTS_BOUNCE;

  polygon_t *star = malloc(sizeof(polygon_t));
  star->centroid = (vector_t){.x = CENTER.x, .y = CENTER.y};
  star->velocity = (vector_t){.x = VEL.x, .y = VEL.y};
  star->red = PURPLE_R;
  star->blue = PURPLE_B;
  star->green = PURPLE_G;
  star->dt = time_since_last_tick();
  star->points =
      draw_star(star->centroid, STAR_RADIUS_BOUNCE, NUM_POINTS_BOUNCE);
  list_add(state->list, (void *)star);

  return state;
}

void emscripten_main(state_t *state) {
  sdl_clear();
  for (int i = 0; i < list_size(state->list); i++) {
    polygon_t *curr = (polygon_t *)list_get(state->list, i);
    curr->dt += time_since_last_tick();
    curr->velocity = check_bounce(curr->points, curr->velocity);
    curr->centroid = vec_add(curr->centroid, curr->velocity);
    curr->points =
        draw_star(curr->centroid, STAR_RADIUS_BOUNCE, NUM_POINTS_BOUNCE);

    polygon_rotate(curr->points, (double)ROTATION_ANGLE * curr->dt,
                   curr->centroid);

    rgb_color_t col = {PURPLE_R, PURPLE_G, PURPLE_B};
    sdl_draw_polygon(curr->points, col);
  }
  sdl_show();
}

void emscripten_free(state_t *state) {
  list_free(state->list);
  free(state);
}
