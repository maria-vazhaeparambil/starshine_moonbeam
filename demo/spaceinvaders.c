#include "body.h"
#include "forces.h"
#include "info.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include "star_body.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

const vector_t WINDOW = (vector_t){.x = 945, .y = 975};

const size_t NUM_BODIES = 8;
const size_t NUM_LINES = 3;

const rgb_color_t GREY = {.r = 0.5, .b = 0.5, .g = 0.5};
const rgb_color_t NEON_GREEN = {.r = 0.0, .b = 0.0, .g = 1.0};

const size_t INVADER_TYPE = 1;
const size_t INVADER_POINTS = 180;
const size_t INVADER_RADIUS = 50;
const size_t INVADER_RADIUS_IN = 20;
const double INVADER_ANGLE = 180.0;
const size_t INVADER_GAP_HORIZ = 20;
const size_t INVADER_GAP_VERT = 10;
const double INVADER_SPEED = 100.0;

const size_t PLAYER_TYPE = 0;
const size_t PLAYER_LENGTH = 50;
const size_t PLAYER_HEIGHT = 10;
const size_t PLAYER_SHIFT = 5;
const double PLAYER_SPEED = 200.0;

const size_t PROJ_TYPE = 2;
const size_t PROJ_POINTS = 4;
const size_t PROJ_WIDTH = 10;
const size_t PROJ_HEIGHT = 30;
const double PROJ_SPEED = 400.0;

const size_t SPAWN_TIME_SPACE = 200;

const double CIRC_ANG = 360.0;
const double ROT_CONV = M_PI / 180.0;

typedef struct state {
  scene_t *scene;
  size_t time_elapsed;
} state_t;

size_t get_rand_invader(state_t *s) {
  size_t pick = 0;
  while (scene_counter(s->scene) < NUM_LINES * NUM_BODIES &&
         get_typ((info_t *)body_get_info(scene_get_body(s->scene, pick))) !=
             INVADER_TYPE) {
    pick = (size_t)(((float)rand() / (float)RAND_MAX) * scene_bodies(s->scene));
  }
  return pick;
}

list_t *draw_invader(vector_t centroid) {
  size_t n = INVADER_POINTS;
  list_t *new_points = list_init(n + 1, free);
  double shift = (CIRC_ANG - INVADER_ANGLE) / 2.0;
  for (size_t i = 0; i < n; i++) {
    vector_t *out = malloc(sizeof(vector_t));
    out->x =
        INVADER_RADIUS * cos(ROT_CONV * (INVADER_ANGLE / n * i)) + centroid.x;
    out->y =
        INVADER_RADIUS * sin(ROT_CONV * (INVADER_ANGLE / n * i)) + centroid.y;
    list_add(new_points, out);
  }
  vector_t *in = malloc(sizeof(vector_t));
  in->x = INVADER_RADIUS_IN * cos(ROT_CONV * (INVADER_ANGLE / n - shift)) +
          centroid.x;
  in->y = INVADER_RADIUS_IN * sin(ROT_CONV * (INVADER_ANGLE / n - shift)) +
          centroid.y;
  list_add(new_points, in);
  return new_points;
}

list_t *draw_projectile(body_t *body, double up) {
  size_t n = PROJ_POINTS;
  list_t *new_points = list_init(n, free);
  vector_t center = body_get_centroid(body);
  vector_t *bottom_right = malloc(sizeof(vector_t));
  bottom_right->x = center.x - PROJ_WIDTH / 2.0;
  bottom_right->y = center.y;
  vector_t *top_right = malloc(sizeof(vector_t));
  top_right->x = center.x - PROJ_WIDTH / 2.0;
  top_right->y = center.y + up * PROJ_HEIGHT;
  vector_t *top_left = malloc(sizeof(vector_t));
  top_left->x = center.x + PROJ_WIDTH / 2.0;
  top_left->y = center.y + up * PROJ_HEIGHT;
  vector_t *bottom_left = malloc(sizeof(vector_t));
  bottom_left->x = center.x + PROJ_WIDTH / 2.0;
  bottom_left->y = center.y;
  list_add(new_points, bottom_right);
  list_add(new_points, top_right);
  list_add(new_points, top_left);
  list_add(new_points, bottom_left);
  return new_points;
}

void gen_projectile(state_t *s, body_t *body, double up) {
  body_t *projectile =
      body_init_with_info(draw_projectile(body, up), 1.0, body_get_color(body),
                          info_init(PROJ_TYPE), (free_func_t)info_free);
  if (get_typ((info_t *)body_get_info(body)) == INVADER_TYPE) {
    create_destructive_collision(s->scene, scene_get_body(s->scene, 0),
                                 projectile);
  } else {
    for (size_t i = 0; i < scene_bodies(s->scene); i++) {
      if (get_typ((info_t *)body_get_info(scene_get_body(s->scene, i))) ==
          INVADER_TYPE) {
        create_destructive_collision(s->scene, scene_get_body(s->scene, i),
                                     projectile);
      }
    }
  }
  scene_add_body(s->scene, projectile);
  body_set_velocity(projectile, (vector_t){.x = 0.0, .y = up * PROJ_SPEED});
}

list_t *draw_player(vector_t centroid) {
  size_t n = CIRC_ANG;
  list_t *new_points = list_init(n, free);
  for (size_t i = 0; i < n; i++) {
    vector_t *out = malloc(sizeof(vector_t));
    out->x = PLAYER_LENGTH * cos(ROT_CONV * (CIRC_ANG / n * i)) + centroid.x;
    out->y = PLAYER_HEIGHT * sin(ROT_CONV * (CIRC_ANG / n * i)) + centroid.y;
    list_add(new_points, out);
  }
  return new_points;
}

void gen_bodies(state_t *state) {
  body_t *player = body_init_with_info(
      draw_player((vector_t){WINDOW.x / 2.0, PLAYER_HEIGHT + PLAYER_SHIFT}),
      1.0, NEON_GREEN, info_init(PLAYER_TYPE), (free_func_t)info_free);
  scene_add_body(state->scene, player);
  for (size_t i = 0; i < NUM_BODIES; i++) {
    for (size_t j = 0; j < NUM_LINES; j++) {
      vector_t center = (vector_t){
          .x = INVADER_RADIUS + (INVADER_RADIUS * 2 + INVADER_GAP_HORIZ) * i,
          .y = WINDOW.y -
               (INVADER_RADIUS +
                (INVADER_RADIUS + INVADER_RADIUS_IN + INVADER_GAP_VERT) * j)};
      info_t *info = info_init(INVADER_TYPE);
      scene_add_body(state->scene,
                     body_init_with_info(draw_invader(center), 1.0, GREY, info,
                                         (free_func_t)info_free));
      body_set_velocity(scene_get_body(state->scene, NUM_LINES * i + j + 1),
                        (vector_t){.x = INVADER_SPEED, .y = 0.0});
    }
  }
}

void check_wall_invader(state_t *s) {
  bool invaders_left = false;
  for (size_t i = 0; i < scene_bodies(s->scene); i++) {
    if (get_typ((info_t *)body_get_info(scene_get_body(s->scene, i))) ==
        INVADER_TYPE) {
      invaders_left = true;
      double new_x = body_get_centroid(scene_get_body(s->scene, i)).x;
      double new_y = body_get_centroid(scene_get_body(s->scene, i)).y;
      if (new_x + INVADER_RADIUS > WINDOW.x) {
        new_x = WINDOW.x - INVADER_RADIUS;
        new_y = new_y - NUM_LINES * (INVADER_RADIUS + INVADER_RADIUS_IN +
                                     INVADER_GAP_VERT);
        body_set_velocity(
            scene_get_body(s->scene, i),
            vec_negate(body_get_velocity(scene_get_body(s->scene, i))));
      }
      if (new_x - INVADER_RADIUS < 0) {
        new_x = INVADER_RADIUS;
        new_y = new_y - NUM_LINES * (INVADER_RADIUS + INVADER_RADIUS_IN +
                                     INVADER_GAP_VERT);
        body_set_velocity(
            scene_get_body(s->scene, i),
            vec_negate(body_get_velocity(scene_get_body(s->scene, i))));
      } else if (new_y - INVADER_RADIUS_IN <= 0) {
        exit(0);
      }
      body_set_centroid(scene_get_body(s->scene, i), (vector_t){new_x, new_y});
    }
  }
  if (!invaders_left) {
    exit(0);
  }
}

void check_wall_player(state_t *s) {
  double new_x = body_get_centroid(scene_get_body(s->scene, 0)).x;
  if (new_x + PLAYER_LENGTH > WINDOW.x) {
    body_set_velocity(scene_get_body(s->scene, 0), VEC_ZERO);
    body_set_centroid(scene_get_body(s->scene, 0),
                      (vector_t){.x = WINDOW.x - PLAYER_LENGTH,
                                 .y = PLAYER_HEIGHT + PLAYER_SHIFT});
  }
  if (new_x - PLAYER_LENGTH < 0) {
    body_set_velocity(scene_get_body(s->scene, 0), VEC_ZERO);
    body_set_centroid(
        scene_get_body(s->scene, 0),
        (vector_t){.x = PLAYER_LENGTH, .y = PLAYER_HEIGHT + PLAYER_SHIFT});
  }
}

void on_key(state_t *s, char key, key_event_type_t type, double held_time) {
  if (type == KEY_PRESSED) {
    switch (key) {
    case RIGHT_ARROW:
      body_set_xvelocity(scene_get_body(s->scene, 0), PLAYER_SPEED);
      break;
    case LEFT_ARROW:
      body_set_xvelocity(scene_get_body(s->scene, 0), -1 * PLAYER_SPEED);
      break;
    case ' ':
      gen_projectile(s, scene_get_body(s->scene, 0), 1.0);
      break;
    }
  }

  if (type == KEY_RELEASED) {
    switch (key) {
    case RIGHT_ARROW:
      body_set_velocity(scene_get_body(s->scene, 0), VEC_ZERO);
      break;
    case LEFT_ARROW:
      body_set_velocity(scene_get_body(s->scene, 0), VEC_ZERO);
      break;
    }
  }
}

state_t *emscripten_init() {
  vector_t min = (vector_t){.x = 0, .y = 0};
  vector_t max = WINDOW;
  sdl_init(min, max);
  sdl_on_key(on_key);
  state_t *state = malloc(sizeof(state_t));
  state->scene = scene_init();
  state->time_elapsed = 0.0;
  gen_bodies(state);
  return state;
}

void emscripten_main(state_t *state) {
  sdl_render_scene(state->scene);
  check_wall_invader(state);
  check_wall_player(state);
  scene_tick(state->scene, time_since_last_tick());
  state->time_elapsed += 1;
  if (state->time_elapsed % SPAWN_TIME_SPACE == 0) {
    gen_projectile(state, scene_get_body(state->scene, get_rand_invader(state)),
                   -1.0);
  }
}

void emscripten_free(state_t *state) {
  scene_free(state->scene);
  free(state);
}
