#include "pacman_util.h"
#include "polygon.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include "state.h"
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

const vector_t WINDOW = (vector_t){.x = 1000, .y = 500};
const vector_t CENTER = (vector_t){.x = 500, .y = 250};
const vector_t START = (vector_t){.x = 200, .y = 250};

const size_t PEL_INIT = 9;
const size_t PEL_RADIUS = 4;
const size_t PAC_RADIUS = 50;
const size_t EAT_RADIUS = 23;
const size_t PEL_MASS = 1;
const size_t PAC_MASS = 50;

const rgb_color_t YELLOW = (rgb_color_t){.r = 1.0, .g = 1.0, .b = 0.0};

const size_t SPAWN_TIME_SPACE = 20;
const double SPEED_INIT = 50;
const double GROWTH_FACTOR = 100.0;

const double RIGHT = 0;
const double UP = M_PI / 2;
const double LEFT = M_PI;
const double DOWN = 3 * M_PI / 2;

typedef struct state {
  scene_t *scene;
  size_t time_elapsed;
  size_t pellets_eaten;
} state_t;

void generate_pellet(state_t *s) {
  double rand_x = get_rand_pos(WINDOW.x - 2 * PEL_RADIUS) + PEL_RADIUS;
  double rand_y = get_rand_pos(WINDOW.y - 2 * PEL_RADIUS) + PEL_RADIUS;
  vector_t rand_cent = (vector_t){.x = rand_x, .y = rand_y};
  scene_add_body(s->scene, body_init(draw_pellet(PEL_RADIUS, rand_cent),
                                     PEL_MASS, YELLOW));
}

double dist_centers(vector_t center_pac, vector_t center_pel) {
  return sqrt((center_pel.x - center_pac.x) * (center_pel.x - center_pac.x) +
              (center_pel.y - center_pac.y) * (center_pel.y - center_pac.y));
}
void pell_in_pac(state_t *s) {
  body_t *pac = scene_get_body(s->scene, 0);
  vector_t pac_center = body_get_centroid(pac);
  for (int i = 1; i < scene_bodies(s->scene); i++) {
    vector_t pel_center = body_get_centroid(scene_get_body(s->scene, i));
    double d = dist_centers(pac_center, pel_center);
    if (d < EAT_RADIUS) {
      scene_remove_body(s->scene, i);
      s->pellets_eaten++;
    }
  }
}

void direction(state_t *s, double angle) {
  body_t *pacman = scene_get_body(s->scene, 0);
  body_set_rotation(pacman, angle);
}

void on_key(state_t *s, char key, key_event_type_t type, double held_time) {
  double value = SPEED_INIT;
  if (type == KEY_PRESSED) {
    switch (key) {
    case UP_ARROW:
      direction(s, UP);
      body_set_xvelocity(scene_get_body(s->scene, 0), 0);
      body_set_acceleration(scene_get_body(s->scene, 0),
                            (vector_t){0, held_time * GROWTH_FACTOR + value});
      break;
    case DOWN_ARROW:
      direction(s, DOWN);
      body_set_xvelocity(scene_get_body(s->scene, 0), 0);
      body_set_acceleration(
          scene_get_body(s->scene, 0),
          (vector_t){0, -1 * held_time * GROWTH_FACTOR - value});
      break;
    case RIGHT_ARROW:
      direction(s, RIGHT);
      body_set_yvelocity(scene_get_body(s->scene, 0), 0);
      body_set_acceleration(scene_get_body(s->scene, 0),
                            (vector_t){held_time * GROWTH_FACTOR + value, 0});
      break;
    case LEFT_ARROW:
      direction(s, LEFT);
      body_set_yvelocity(scene_get_body(s->scene, 0), 0);
      body_set_acceleration(
          scene_get_body(s->scene, 0),
          (vector_t){-1 * held_time * GROWTH_FACTOR - value, 0});
      break;
    }
  }

  if (type == KEY_RELEASED) {
    switch (key) {
    case UP_ARROW:
      body_set_velocity(scene_get_body(s->scene, 0), (vector_t){0, value});
      body_set_acceleration(scene_get_body(s->scene, 0), VEC_ZERO);
      break;
    case DOWN_ARROW:
      body_set_velocity(scene_get_body(s->scene, 0), (vector_t){0, -1 * value});
      body_set_acceleration(scene_get_body(s->scene, 0), VEC_ZERO);
      break;
    case RIGHT_ARROW:
      body_set_velocity(scene_get_body(s->scene, 0), (vector_t){value, 0});
      body_set_acceleration(scene_get_body(s->scene, 0), VEC_ZERO);
      break;
    case LEFT_ARROW:
      body_set_velocity(scene_get_body(s->scene, 0), (vector_t){-1 * value, 0});
      body_set_acceleration(scene_get_body(s->scene, 0), VEC_ZERO);
      break;
    }
  }
}

state_t *emscripten_init() {
  vector_t min = (vector_t){.x = 0, .y = 0};
  vector_t max = WINDOW;
  sdl_init(min, max);
  sdl_on_key(on_key);
  state_t *s = malloc(sizeof(state_t));
  s->time_elapsed = 0.0;
  s->pellets_eaten = 0;
  s->scene = scene_init();
  scene_add_body(s->scene,
                 body_init(draw_pacman(PAC_RADIUS, START), PAC_MASS, YELLOW));
  body_set_velocity(scene_get_body(s->scene, 0), VEC_ZERO);
  for (int i = 0; i < PEL_INIT; i++) {
    generate_pellet(s);
  }
  return s;
}

void check_wall(state_t *s) {
  double new_x = body_get_centroid(scene_get_body(s->scene, 0)).x;
  double new_y = body_get_centroid(scene_get_body(s->scene, 0)).y;
  if (new_x >= WINDOW.x) {
    new_x -= WINDOW.x;
  } else if (new_y >= WINDOW.y) {
    new_y -= WINDOW.y;
  } else if (new_x <= 0) {
    new_x += WINDOW.x;
  } else if (new_y <= 0) {
    new_y += WINDOW.y;
  } else {
    return;
  }
  body_set_centroid(scene_get_body(s->scene, 0), (vector_t){new_x, new_y});
}

void emscripten_main(state_t *s) {
  sdl_render_scene(s->scene);
  scene_tick(s->scene, time_since_last_tick());
  check_wall(s);
  pell_in_pac(s);
  s->time_elapsed += 1;
  if (s->time_elapsed % SPAWN_TIME_SPACE == 0) {
    generate_pellet(s);
  }
}

void emscripten_free(state_t *s) {
  scene_free(s->scene);
  free(s);
}
