#include "body.h"
#include "color.h"
#include "info.h"
#include "list.h"
#include "polygon.h"
#include "vector.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_image.h>

typedef struct body {
  rgb_color_t color;
  double mass;
  vector_t position;
  vector_t velocity;
  vector_t acceleration;
  list_t *shape;
  vector_t total_force;
  vector_t total_impulse;
  vector_t centroid;
  double curr_angle;
  void *info;
  bool body_remove;
  double height;
  double width;
  bool lose;
  bool win;
  bool fan;
  bool ground;
  double pull_mass;
  SDL_Texture *texture;
  free_func_t info_freer;
} body_t;

body_t *body_init(list_t *shape, double mass, rgb_color_t color) {
  body_t *b_new = malloc(sizeof(body_t));
  assert(b_new != NULL);
  b_new->shape = shape;
  b_new->mass = mass;
  b_new->color = color;
  b_new->centroid = (vector_t)(polygon_centroid(shape));
  b_new->curr_angle = 0;
  b_new->velocity = VEC_ZERO;
  b_new->acceleration = VEC_ZERO;
  b_new->total_force = VEC_ZERO;
  b_new->total_impulse = VEC_ZERO;
  b_new->info_freer = NULL;
  b_new->body_remove = false;
  b_new->lose = false;
  b_new->win = false;
  b_new->fan = false;
  b_new->texture = NULL;
  b_new->ground = false;
  b_new->pull_mass = 0.0;
  return b_new;
}

void body_free(body_t *body) {
  if(body->texture!=NULL){
    SDL_DestroyTexture(body->texture);
  }
  if (body->info_freer != NULL) {
    body->info_freer(body->info);
  }
  list_free(body->shape);
  free(body);
}

list_t *body_get_shape(body_t *body) {
  list_t *return_shape = list_init(list_size(body->shape), free);
  for (int i = 0; i < list_size(body->shape); i++) {
    vector_t *v = malloc(sizeof(vector_t));
    v->x = ((vector_t *)list_get(body->shape, i))->x;
    v->y = ((vector_t *)list_get(body->shape, i))->y;
    list_add(return_shape, v);
  }
  return return_shape;
}

void body_set_color(body_t *body, rgb_color_t col) { body->color = col; }

double body_get_mass(body_t *body) { return body->mass; }

vector_t body_get_centroid(body_t *body) { return body->centroid; }

vector_t body_get_velocity(body_t *body) { return body->velocity; }

vector_t body_get_acceleration(body_t *body) { return body->acceleration; }

vector_t body_get_total_force(body_t *body) { return body->total_force; }

vector_t body_get_total_impulse(body_t *body) { return body->total_impulse; }

rgb_color_t body_get_color(body_t *body) { return body->color; }

void body_set_centroid(body_t *body, vector_t x) {
  polygon_translate(body->shape, vec_subtract(x, body->centroid));
  body->centroid = x;
}

void body_set_velocity(body_t *body, vector_t v) { body->velocity = v; }

void body_set_xvelocity(body_t *body, double x) { body->velocity.x = x; }

void body_set_yvelocity(body_t *body, double y) { body->velocity.y = y; }

void body_set_acceleration(body_t *body, vector_t a) { body->acceleration = a; }

void body_set_rotation(body_t *body, double angle) {
  double delta_angle = angle - body->curr_angle;
  body->curr_angle = angle;
  polygon_rotate(body->shape, delta_angle, body->centroid);
}

void body_tick(body_t *body, double dt) {

  vector_t v_initial = body->velocity;

  vector_t imp_v = vec_multiply(1 / body->mass, body->total_impulse);

  vector_t v_final = v_initial;
  
  v_final = vec_add(v_final, vec_multiply(dt, body->acceleration));

  v_final = vec_add(v_final, imp_v);

  vector_t move = vec_multiply(0.5, vec_add(v_initial, v_final));
  move = vec_multiply(dt, move);

  body_set_centroid(body, vec_add(body->centroid, move));
  body_set_velocity(body, v_final);
  body->total_force = VEC_ZERO;
  body->total_impulse = VEC_ZERO;
  body->ground = false;
  body->pull_mass = 0.0;
}

void body_add_force(body_t *body, vector_t force) {
  body->total_force = vec_add(body->total_force, force);
  
  double accel_x = body->total_force.x / body->mass;
  double accel_y = body->total_force.y / body->mass;
  body->acceleration = (vector_t){accel_x, accel_y};
}

void body_add_impulse(body_t *body, vector_t impulse) {
  body->total_impulse = vec_add(body->total_impulse, impulse);
}

body_t *body_init_with_info(list_t *shape, double mass, rgb_color_t color,
                            void *info, free_func_t info_freer) {
  body_t *b_new = malloc(sizeof(body_t));
  assert(b_new != NULL);
  b_new->shape = shape;
  b_new->mass = mass;
  b_new->color = color;
  b_new->centroid = (vector_t)(polygon_centroid(shape));
  b_new->curr_angle = 0;
  b_new->velocity = VEC_ZERO;
  b_new->acceleration = VEC_ZERO;
  b_new->total_force = VEC_ZERO;
  b_new->total_impulse = VEC_ZERO;
  b_new->info = info;
  b_new->info_freer = info_freer;
  b_new->body_remove = false;
  b_new->lose = false;
  b_new->win = false;
  b_new->fan = false;
  b_new->ground = false;
  b_new->pull_mass = 0.0;
  b_new->texture = NULL;
  return b_new;
}

body_t *body_init_more_info(list_t *shape, double mass, rgb_color_t color, double width, double height) {
  body_t *b_new = malloc(sizeof(body_t));
  assert(b_new != NULL);
  b_new->shape = shape;
  b_new->mass = mass;
  b_new->color = color;
  b_new->centroid = (vector_t)(polygon_centroid(shape));
  b_new->curr_angle = 0;
  b_new->velocity = VEC_ZERO;
  b_new->acceleration = VEC_ZERO;
  b_new->total_force = VEC_ZERO;
  b_new->total_impulse = VEC_ZERO;
  b_new->info_freer = NULL;
  b_new->body_remove = false;
  b_new->width = width;
  b_new->height = height;
  b_new->lose = false;
  b_new->win = false;
  b_new->fan = false;
  b_new->ground = false;
  b_new->pull_mass = 0.0;
  b_new->texture = NULL;
  return b_new;
}

void body_lose(body_t *body) { body->lose = true; }
void body_win(body_t *body) { body->win = true; }
void body_fan(body_t *body, bool change) { body->fan = change; }

bool body_get_lose(body_t *body) { return body->lose; }
bool body_get_win(body_t *body) { return body->win; }
bool body_get_fan(body_t *body) { return body->fan; }

void body_set_ground(body_t *body, bool change) {body->ground = change;}
bool body_get_ground(body_t *body) {return body->ground;}

void body_set_pull_mass(body_t *body, double mass) {body->pull_mass = mass;}
double body_get_pull_mass(body_t *body) {return body->pull_mass;}

SDL_Texture *body_get_texture(body_t *body) { return body->texture; }
void body_set_texture(body_t *body, SDL_Texture *text) { body->texture = text; }

void body_remove(body_t *body) { body->body_remove = true; }

bool body_is_removed(body_t *body) { return body->body_remove; }

void *body_get_info(body_t *body) { return body->info; }

double body_get_height(body_t *body) { return body->height; }

double body_get_width(body_t *body) { return body->width; }

free_func_t body_get_info_freer(body_t *body) { return body->info_freer; }


