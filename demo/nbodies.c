#include "body.h"
#include "forces.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include "star_body.h"

const double GRAV_CONST = 500;
const size_t NUM_BODIES = 50;
const rgb_color_t color = {.r = 0.5, .b = 0.5, .g = 0.5};
const size_t STAR_RAD_N = 20;
const double MASS_RANGE = 50;
const double MIN_MASS = 20;
const double MIN_VELOCITY = 5.0;
const double RANGE_VELOCITY = 5.0;
const double MIN_RAD = 10.0;

const vector_t WINDOW = (vector_t){.x = 1000, .y = 500};

typedef struct state {
  scene_t *scene;
} state_t;

vector_t get_rand_centroid() {
  vector_t centroid = {
      .x = (double)(((float)rand() / (float)RAND_MAX) * (WINDOW.x / 2)) +
           (WINDOW.x / 4),
      .y = (double)(((float)rand() / (float)RAND_MAX) * (WINDOW.y / 2)) +
           WINDOW.y / 4};
  return centroid;
}

double get_rand_rad() {
  return ((float)rand() / (float)RAND_MAX) * STAR_RAD_N + MIN_RAD;
}

double get_rand_mass() {
  double mass =
      (double)(((float)rand() / (float)RAND_MAX) * MASS_RANGE + MIN_MASS);
  return mass;
}

rgb_color_t get_rand_color() {
  rgb_color_t col = {
      .r = get_rand_col(), .b = get_rand_col(), .g = get_rand_col()};
  return col;
}

vector_t get_rand_velocity() {
  vector_t velocity = {
      .x = (double)(((float)rand() / (float)RAND_MAX) * (RANGE_VELOCITY)) +
           MIN_VELOCITY,
      (double)(((float)rand() / (float)RAND_MAX) * (RANGE_VELOCITY)) +
          MIN_VELOCITY};
  if ((int)velocity.x % 2 == 0) {
    velocity.x = (-1) * velocity.x;
  }
  if ((int)velocity.y % 2 == 0) {
    velocity.y = (-1) * velocity.y;
  }
  return velocity;
}

void gen_bodies(state_t *state) {
  for (size_t i = 0; i < NUM_BODIES; i++) {
    list_t *shape = draw_star(get_rand_centroid(), get_rand_rad(), 4);
    body_t *body = body_init(shape, get_rand_mass(), get_rand_color());
    body_set_velocity(body, get_rand_velocity());
    scene_add_body(state->scene, body);
  }
}

state_t *emscripten_init() {
  vector_t min = (vector_t){.x = 0, .y = 0};
  vector_t max = WINDOW;
  sdl_init(min, max);
  state_t *state = malloc(sizeof(state_t));
  state->scene = scene_init();
  gen_bodies(state);
  for (size_t i = 0; i < NUM_BODIES; i++) {
    for (size_t j = i + 1; j < NUM_BODIES; j++) {
      create_newtonian_gravity(state->scene, GRAV_CONST,
                               scene_get_body(state->scene, i),
                               scene_get_body(state->scene, j));
    }
  }
  return state;
}

void emscripten_main(state_t *state) {
  sdl_render_scene(state->scene);
  scene_tick(state->scene, time_since_last_tick());
}

void emscripten_free(state_t *state) {
  scene_free(state->scene);
  free(state);
}
