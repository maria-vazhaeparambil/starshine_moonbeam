#include "body.h"
#include "forces.h"
#include "info.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include <math.h>

const vector_t WINDOW = (vector_t){.x = 1048, .y = 975};

const size_t NUM_BRICKS = 10;
const size_t NUM_LINES = 3;

const size_t PLAYER_TYPE = 0;
const size_t PLAYER_POINTS = 4;
const size_t PLAYER_LENGTH = 90;
const size_t PLAYER_HEIGHT = 30;
const size_t PLAYER_GAP = 4;
const size_t PLAYER_SHIFT = 0;
const double PLAYER_SPEED = 500.0;
const double PLAYER_MASS = 50;

const size_t BRICK_TYPE = 1;
const size_t BRICK_POINTS = 4;
const size_t BRICK_LENGTH = 96;
const size_t BRICK_HEIGHT = 30;
const size_t BRICK_MASS = 60;
const size_t GAP_HORIZ = 8;
const size_t GAP_VERT = 8;

const size_t BALL_TYPE = 2;
const size_t BALL_POINTS = 360;
const size_t BALL_RADIUS = 10;
const size_t BALL_MASS = 5;
const size_t BALL_INIT = 100;
const vector_t VEC_INIT = (vector_t){.x = 400, .y = 400};

const size_t WALL_POINTS = 3;
const size_t WALL_TYPE = 3;

const rgb_color_t RED = (rgb_color_t){.r = 1.0, .g = 0.0, .b = 0.0};
const rgb_color_t BLACK = (rgb_color_t){.r = 0.0, .g = 0.0, .b = 0.0};
const size_t GREEN_CHANGE = 3;
const size_t BLUE_CHANGE = 6;
const size_t RED_CHANGE = 9;

const double ROTAT_CONV = M_PI / 180.0;
const double CIRC_ANG = 360.0;

typedef struct state {
  scene_t *scene;
  size_t time_elapsed;
} state_t;

rgb_color_t generate_col(size_t index) {
  if (index < GREEN_CHANGE) {
    return (rgb_color_t){.r = 1.0, .g = index % GREEN_CHANGE * 0.5, .b = 0.0};
  } else if (index < BLUE_CHANGE) {
    return (rgb_color_t){
        .r = 0.0, .g = 1.0, .b = index % (BLUE_CHANGE - GREEN_CHANGE) * 0.5};
  } else if (index < RED_CHANGE) {
    return (rgb_color_t){
        .r = index % (RED_CHANGE - BLUE_CHANGE) * 0.5, .g = 0.0, .b = 1.0};
  } else {
    return (rgb_color_t){.r = 1.0, .g = 0.0, .b = 0.5};
  }
}

list_t *draw_ball(size_t radius, vector_t centroid) {
  size_t n = BALL_POINTS;
  list_t *new_points = list_init(n, free);
  for (size_t i = 0; i < n; i++) {
    vector_t *out = malloc(sizeof(vector_t));
    *out = (vector_t){
        .x = radius * cos(ROTAT_CONV * CIRC_ANG / n * i) + centroid.x,
        .y = radius * sin(ROTAT_CONV * CIRC_ANG / n * i) + centroid.y};
    list_add(new_points, out);
  }
  return new_points;
}

list_t *draw_rect(vector_t center, double length, double height) {
  size_t n = BRICK_POINTS;
  list_t *new_points = list_init(n, free);
  vector_t *bottom_right = malloc(sizeof(vector_t));
  bottom_right->x = center.x - length / 2.0;
  bottom_right->y = center.y;
  vector_t *top_right = malloc(sizeof(vector_t));
  top_right->x = center.x - length / 2.0;
  top_right->y = center.y + height;
  vector_t *top_left = malloc(sizeof(vector_t));
  top_left->x = center.x + length / 2.0;
  top_left->y = center.y + height;
  vector_t *bottom_left = malloc(sizeof(vector_t));
  bottom_left->x = center.x + length / 2.0;
  bottom_left->y = center.y;
  list_add(new_points, bottom_right);
  list_add(new_points, top_right);
  list_add(new_points, top_left);
  list_add(new_points, bottom_left);
  return new_points;
}

list_t *draw_left_wall() {
  list_t *left = list_init(WALL_POINTS, free);
  vector_t *left_bottom = malloc(sizeof(vector_t));
  left_bottom->x = 0;
  left_bottom->y = 0;
  vector_t *left_top = malloc(sizeof(vector_t));
  left_top->x = 0;
  left_top->y = WINDOW.y;
  vector_t *rand = malloc(sizeof(vector_t));
  rand->x = -1;
  rand->y = WINDOW.y / 2;
  list_add(left, left_bottom);
  list_add(left, left_top);
  list_add(left, rand);
  return left;
}

list_t *draw_right_wall() {
  list_t *right = list_init(WALL_POINTS, free);
  vector_t *right_bottom = malloc(sizeof(vector_t));
  right_bottom->x = WINDOW.x;
  right_bottom->y = 0;
  vector_t *right_top = malloc(sizeof(vector_t));
  right_top->x = WINDOW.x;
  right_top->y = WINDOW.y;
  vector_t *rand = malloc(sizeof(vector_t));
  rand->x = WINDOW.x + 1;
  rand->y = WINDOW.y / 2;
  list_add(right, right_bottom);
  list_add(right, right_top);
  list_add(right, rand);
  return right;
}

list_t *draw_top_wall() {
  list_t *top = list_init(WALL_POINTS, free);
  vector_t *top_left = malloc(sizeof(vector_t));
  top_left->x = 0;
  top_left->y = WINDOW.y;
  vector_t *top_right = malloc(sizeof(vector_t));
  top_right->x = 0;
  top_right->y = WINDOW.y;
  vector_t *rand = malloc(sizeof(vector_t));
  rand->x = WINDOW.x / 2;
  rand->y = -1;
  list_add(top, top_left);
  list_add(top, top_right);
  list_add(top, rand);
  return top;
}

void check_wall(state_t *s) {
  double new_x = body_get_centroid(scene_get_body(s->scene, 0)).x;
  if (new_x + (PLAYER_LENGTH / 2) > WINDOW.x) {
    body_set_velocity(scene_get_body(s->scene, 0), VEC_ZERO);
    body_set_centroid(scene_get_body(s->scene, 0),
                      (vector_t){.x = WINDOW.x - (PLAYER_LENGTH / 2),
                                 .y = PLAYER_HEIGHT + PLAYER_SHIFT});
  }
  if (new_x - (PLAYER_LENGTH / 2) < 0) {
    body_set_velocity(scene_get_body(s->scene, 0), VEC_ZERO);
    body_set_centroid(scene_get_body(s->scene, 0),
                      (vector_t){.x = (PLAYER_LENGTH / 2),
                                 .y = PLAYER_HEIGHT + PLAYER_SHIFT});
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

void init_scene(state_t *state) {
  state->time_elapsed = 0;
  state->scene = scene_init();
  body_t *player = body_init_with_info(
      draw_rect((vector_t){WINDOW.x / 2, PLAYER_GAP + PLAYER_HEIGHT / 2},
                BRICK_LENGTH, BRICK_HEIGHT),
      PLAYER_MASS, RED, info_init(PLAYER_TYPE), (free_func_t)info_free);
  scene_add_body(state->scene, player);
  body_t *ball = body_init_with_info(
      draw_ball(BALL_RADIUS,
                (vector_t){WINDOW.x / 2, BALL_INIT + PLAYER_HEIGHT / 2}),
      BALL_MASS, RED, info_init(BALL_TYPE), (free_func_t)info_free);
  body_set_velocity(ball, VEC_INIT);
  scene_add_body(state->scene, ball);
  body_t *left_wall = body_init_with_info(
      draw_left_wall(), INFINITY, (rgb_color_t){.r = 1.0, .g = 1.0, .b = 1.0},
      info_init(WALL_TYPE), (free_func_t)info_free);
  scene_add_body(state->scene, left_wall);
  body_t *right_wall = body_init_with_info(
      draw_right_wall(), INFINITY, (rgb_color_t){.r = 1.0, .g = 1.0, .b = 1.0},
      info_init(WALL_TYPE), (free_func_t)info_free);
  scene_add_body(state->scene, right_wall);
  body_t *top_wall = body_init_with_info(
      draw_top_wall(), INFINITY, (rgb_color_t){.r = 1.0, .g = 1.0, .b = 1.0},
      info_init(WALL_TYPE), (free_func_t)info_free);
  scene_add_body(state->scene, top_wall);
  create_physics_collision(state->scene, 1.0, ball, player);
  create_physics_collision(state->scene, 1.0, ball, left_wall);
  create_physics_collision(state->scene, 1.0, ball, right_wall);
  create_physics_collision(state->scene, 1.0, ball, top_wall);
  for (size_t i = 0; i < NUM_BRICKS; i++) {
    rgb_color_t col = generate_col(i);
    for (size_t j = 0; j < NUM_LINES; j++) {
      vector_t center = (vector_t){.x = GAP_HORIZ + BRICK_LENGTH / 2 +
                                        (BRICK_LENGTH + GAP_HORIZ) * i,
                                   .y = WINDOW.y - GAP_VERT - BRICK_HEIGHT -
                                        (BRICK_HEIGHT + GAP_VERT) * j};
      info_t *info = info_init(BRICK_TYPE);
      body_t *brick =
          body_init_with_info(draw_rect(center, BRICK_LENGTH, BRICK_HEIGHT),
                              BRICK_MASS, col, info, (free_func_t)info_free);
      scene_add_body(state->scene, brick);
      create_physics_collision(state->scene, 1.0, ball, brick);
    }
  }
}

void check_bottom_ball(state_t *s) {
  if (body_get_centroid(scene_get_body(s->scene, 1)).y <= 0) {
    scene_free(s->scene);
    init_scene(s);
  }
}

state_t *emscripten_init() {
  vector_t min = (vector_t){.x = 0, .y = 0};
  vector_t max = WINDOW;
  sdl_init(min, max);
  sdl_on_key(on_key);
  state_t *state = malloc(sizeof(state_t));
  init_scene(state);
  return state;
}

void emscripten_main(state_t *state) {
  sdl_render_scene(state->scene);
  check_wall(state);
  scene_tick(state->scene, time_since_last_tick());
  state->time_elapsed += 1;
  check_bottom_ball(state);
}

void emscripten_free(state_t *state) {
  scene_free(state->scene);
  free(state);
}