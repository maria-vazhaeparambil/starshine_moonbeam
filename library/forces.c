#include "forces.h"
#include "body.h"
#include "collision.h"
#include "info.h"
#include "list.h"
#include "scene.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

const size_t MIN_DIST = 5;
const size_t HITS = 3;

typedef struct two_body_param {
  double constant;
  body_t *body1;
  body_t *body2;
} two_body_param_t;

typedef struct two_bodies_param {
  body_t *body1;
  body_t *body2;
  collision_handler_t handler;
  void *aux;
  free_func_t freer;
} two_bodies_param_t;

typedef struct one_body_param {
  double constant;
  body_t *body;
} one_body_param_t;

typedef struct door_param {
  body_t *check_door1;
  body_t *player1;
  body_t *check_door2;
  body_t *player2;
} door_param_t;
typedef struct handle_param {
  size_t hits;
  double constant;
  bool collided;
} handle_param_t;

typedef struct pulley_param {
  body_t *body;
  body_t *pulley1;
  body_t *pulley2;
  double constant;
} pulley_param_t;

void crt_gravity(void *aux) {
  vector_t dist_vec =
      vec_subtract(body_get_centroid(((two_body_param_t *)aux)->body1),
                   body_get_centroid(((two_body_param_t *)aux)->body2));
  double dist = sqrt(dist_vec.x * dist_vec.x + dist_vec.y * dist_vec.y);
  double x_grav = ((two_body_param_t *)aux)->constant *
                  body_get_mass(((two_body_param_t *)aux)->body1) *
                  body_get_mass(((two_body_param_t *)aux)->body2) /
                  (dist * dist) * dist_vec.x / dist;
  double y_grav = ((two_body_param_t *)aux)->constant *
                  body_get_mass(((two_body_param_t *)aux)->body1) *
                  body_get_mass(((two_body_param_t *)aux)->body2) /
                  (dist * dist) * dist_vec.y / dist;
  vector_t gravity = {x_grav, y_grav};
  if (dist >= MIN_DIST) {
    body_add_force(((two_body_param_t *)aux)->body1, vec_negate(gravity));
    body_add_force(((two_body_param_t *)aux)->body2, gravity);
  }
}

void crt_spring(void *aux) {
  vector_t dist_vec =
      vec_subtract(body_get_centroid(((two_body_param_t *)aux)->body1),
                   body_get_centroid(((two_body_param_t *)aux)->body2));
  double x_spring = ((two_body_param_t *)aux)->constant * dist_vec.x;
  double y_spring = ((two_body_param_t *)aux)->constant * dist_vec.y;
  vector_t spring = {x_spring, y_spring};
  body_add_force(((two_body_param_t *)aux)->body1, vec_negate(spring));
}

void crt_drag(void *aux) {
  vector_t drag = {body_get_velocity(((one_body_param_t *)aux)->body).x *
                       ((one_body_param_t *)aux)->constant,
                   body_get_velocity(((one_body_param_t *)aux)->body).y *
                       ((one_body_param_t *)aux)->constant};
  drag = vec_negate(drag);
  body_add_force(((one_body_param_t *)aux)->body, drag);
}

void crt_collision(void *aux) {
  list_t *l1 = body_get_shape(((two_bodies_param_t *)aux)->body1);
  list_t *l2 = body_get_shape(((two_bodies_param_t *)aux)->body2);

  collision_info_t collide = find_collision(l1, l2);
  bool collision = collide.collided;
  vector_t axis = collide.axis;

  list_free(l1);
  list_free(l2);

  if (!collision &&
      ((handle_param_t *)((two_bodies_param_t *)aux)->aux)->collided) {
    ((handle_param_t *)((two_bodies_param_t *)aux)->aux)->collided = false;
  }
  if (collision &&
      !((handle_param_t *)((two_bodies_param_t *)aux)->aux)->collided) {
    ((two_bodies_param_t *)aux)
        ->handler(((two_bodies_param_t *)aux)->body1,
                  ((two_bodies_param_t *)aux)->body2, axis,
                  ((two_bodies_param_t *)aux)->aux);
    ((handle_param_t *)((two_bodies_param_t *)aux)->aux)->collided = true;
  }
}

void crt_plat(void *aux) {
  list_t *l1 = body_get_shape(((two_body_param_t *)aux)->body1);
  list_t *l2 = body_get_shape(((two_body_param_t *)aux)->body2);

  collision_info_t collide = find_collision(l1, l2);
  bool collision = collide.collided;
  
  vector_t axis = collide.axis;

  list_free(l1);
  list_free(l2);
  
  if (collision) {
    if (axis.x == 0.0 && axis.y == -1.0) {
      vector_t player_centroid = body_get_centroid(((two_body_param_t *)aux)->body1);
      vector_t platform_centroid = body_get_centroid(((two_body_param_t *)aux)->body2);

      if ((platform_centroid.y + body_get_height(((two_body_param_t *)aux)->body2)/2) > (player_centroid.y - body_get_height(((two_body_param_t *)aux)->body1)/2) + 1) {
        double y_offset = (platform_centroid.y + body_get_height(((two_body_param_t *)aux)->body2)/2) - (player_centroid.y - body_get_height(((two_body_param_t *)aux)->body1)/2);
        body_set_centroid(((two_body_param_t *)aux)->body1, (vector_t) {player_centroid.x, player_centroid.y + y_offset + 1});
      }
      if (body_get_mass(((two_body_param_t *)aux)->body2) == INFINITY || body_get_velocity(((two_body_param_t *)aux)->body2).y == 0.0){
        vector_t body1_a = body_get_acceleration(((two_body_param_t *)aux)->body1);
        body_set_acceleration(((two_body_param_t *)aux)->body1, (vector_t){.x = body1_a.x, 0.0});
        body_set_yvelocity(((two_body_param_t *)aux)->body1, 0.0);
      }
      else {
        double mass = body_get_mass(((two_body_param_t *)aux)->body1) + body_get_mass(((two_body_param_t *)aux)->body2); 
        double accel = mass * ((two_body_param_t *)aux)->constant;
        vector_t a1 = (vector_t) {.x = body_get_acceleration(((two_body_param_t *)aux)->body1).x, accel};
        vector_t a2 = (vector_t) {.x = body_get_acceleration(((two_body_param_t *)aux)->body2).x, accel};
        body_set_acceleration(((two_body_param_t *)aux)->body1, a1);
        body_set_acceleration(((two_body_param_t *)aux)->body2, a2);
      }
    }
    if ((axis.x == -1.0 && axis.y == 0.0) || (axis.x == 1.0 && axis.y == 0.0)) {
      vector_t player_centroid = body_get_centroid(((two_body_param_t *)aux)->body1);
      vector_t platform_centroid = body_get_centroid(((two_body_param_t *)aux)->body2);
      if (axis.x < 0.0) {
        double x_offset = (platform_centroid.x + body_get_width(((two_body_param_t *)aux)->body2)/2) - (player_centroid.x - body_get_width(((two_body_param_t *)aux)->body1)/2);
        body_set_centroid(((two_body_param_t *)aux)->body1, (vector_t) {player_centroid.x + x_offset + 1, player_centroid.y});
      }
      if (axis.x > 0.0) {
        double x_offset = (player_centroid.x + body_get_width(((two_body_param_t *)aux)->body1)/2) - (platform_centroid.x - body_get_width(((two_body_param_t *)aux)->body2)/2);
        body_set_centroid(((two_body_param_t *)aux)->body1, (vector_t) {player_centroid.x - x_offset - 1, player_centroid.y}); 
      }
      if (body_get_mass(((two_body_param_t *)aux)->body2) == INFINITY) {
        if ((body_get_velocity(((two_body_param_t *)aux)->body1).x < 0 && axis.x < 0) || 
            (body_get_velocity(((two_body_param_t *)aux)->body1).x > 0 && axis.x > 0))
        body_set_xvelocity(((two_body_param_t *)aux)->body1, 0);
      }
      else {
        double vel1 = body_get_velocity(((two_body_param_t *)aux)->body1).x;
        body_set_xvelocity(((two_body_param_t *)aux)->body1, vel1/2);
        body_set_xvelocity(((two_body_param_t *)aux)->body2, vel1/2);
      }
    }
    if (axis.x == 0.0 && axis.y == 1.0) {
      double mass1 = body_get_mass(((two_body_param_t *)aux)->body1);
      double mass2 = body_get_mass(((two_body_param_t *)aux)->body2);
      double masses = mass1 * mass2 / (mass1 + mass2);
      if (mass2 == INFINITY) {
        masses = mass1;
      }
      double constant = masses * (1.0 + 1.0);
      double components = vec_dot(body_get_velocity(((two_body_param_t *)aux)->body2), axis) -
                          vec_dot(body_get_velocity(((two_body_param_t *)aux)->body1), axis);
      body_add_impulse(((two_body_param_t *)aux)->body1, vec_multiply(constant * components, axis));
      if (mass2 != INFINITY) {
        body_add_impulse(((two_body_param_t *)aux)->body2, vec_multiply(-1 * constant * components, axis));
      }
      vector_t player_centroid = body_get_centroid(((two_body_param_t *)aux)->body1);
      vector_t platform_centroid = body_get_centroid(((two_body_param_t *)aux)->body2);
      if ((platform_centroid.y - body_get_height(((two_body_param_t *)aux)->body2)/2) < (player_centroid.y + body_get_height(((two_body_param_t *)aux)->body1)/2) + 1) {
        double y_offset = (platform_centroid.y - body_get_height(((two_body_param_t *)aux)->body2)/2) - (player_centroid.y + body_get_height(((two_body_param_t *)aux)->body1)/2);
        body_set_centroid(((two_body_param_t *)aux)->body1, (vector_t) {player_centroid.x, player_centroid.y + y_offset - 1});
      }
    }
  }
}

void crt_door(void *aux) {
  list_t *l1 = body_get_shape(((door_param_t *)aux)->player1);
  list_t *l2 = body_get_shape(((door_param_t *)aux)->check_door1);

  collision_info_t collide1 = find_collision(l1, l2);
  bool collision1 = collide1.collided;

  list_free(l1);
  list_free(l2);

  list_t *l3 = body_get_shape(((door_param_t *)aux)->player2);
  list_t *l4 = body_get_shape(((door_param_t *)aux)->check_door2);

  collision_info_t collide2 = find_collision(l3, l4);
  bool collision2 = collide2.collided;

  list_free(l3);
  list_free(l4);

  if (collision1 && collision2) {
    body_win(((door_param_t *)aux)->player1);
    body_win(((door_param_t *)aux)->player2);
  }
}

void crt_pulley(void *aux) {
  list_t *l1 = body_get_shape(((pulley_param_t *)aux)->body);
  list_t *l2 = body_get_shape(((pulley_param_t *)aux)->pulley1);
  list_t *l3 = body_get_shape(((pulley_param_t *)aux)->pulley2);

  collision_info_t collide1 = find_collision(l1, l2);
  collision_info_t collide2 = find_collision(l1, l3);

  list_free(l1);
  list_free(l2);
  list_free(l3);

  body_t *coll;
  body_t *no_coll;
  vector_t axis;
  if (collide1.collided) {
    coll = ((pulley_param_t *)aux)->pulley1;
    no_coll = ((pulley_param_t *)aux)->pulley2;
    axis = collide1.axis;
  }
  if (collide2.collided) {
    coll = ((pulley_param_t *)aux)->pulley2;
    no_coll = ((pulley_param_t *)aux)->pulley1;
    axis = collide2.axis;
  }
  if (collide1.collided || collide2.collided) {
    if (axis.x == 0.0 && axis.y == -1.0) {
      vector_t player_centroid = body_get_centroid(((pulley_param_t *)aux)->body);
      vector_t platform_centroid = body_get_centroid(coll);

      if ((platform_centroid.y + body_get_height(coll)/2) > (player_centroid.y - body_get_height(((pulley_param_t *)aux)->body)/2) + 1) {
        double y_offset = (platform_centroid.y + body_get_height(coll)/2) - (player_centroid.y - body_get_height(((pulley_param_t *)aux)->body)/2);
        body_set_centroid(((pulley_param_t *)aux)->body, (vector_t) {player_centroid.x, player_centroid.y + y_offset + 1});
      }

      double mass = body_get_mass(((pulley_param_t *)aux)->body);
      if (mass == INFINITY) {
        body_set_yvelocity(((pulley_param_t *)aux)->body, 0.0);
        body_set_yvelocity(coll, 0.0);
        body_set_yvelocity(no_coll, 0.0);
        body_set_ground(coll, true);
      }
      else {
        body_set_pull_mass(coll, body_get_pull_mass(coll) + mass);
        if (!body_get_ground(coll) && body_get_pull_mass(coll) > body_get_pull_mass(no_coll)) { 
          vector_t body1_a = body_get_acceleration(((pulley_param_t *)aux)->body);
          vector_t body2_a = body_get_acceleration(coll);
          vector_t a = (vector_t) {.x = body2_a.x, body1_a.y};
          body_set_acceleration(coll, a);
          body_set_acceleration(no_coll, vec_negate(a));
        } 
        else if (body_get_pull_mass(coll) > body_get_pull_mass(no_coll)) {
          body_set_yvelocity(((pulley_param_t *)aux)->body, 0.0);
          body_set_acceleration(((pulley_param_t *)aux)->body, VEC_ZERO);
          body_set_acceleration(coll, VEC_ZERO);
          body_set_acceleration(no_coll, VEC_ZERO);
        }
        else {
          body_set_yvelocity(((pulley_param_t *)aux)->body, 0.0);
          body_set_acceleration(((pulley_param_t *)aux)->body, body_get_acceleration(coll));
        }
      }
    }
    if ((axis.x == -1.0 && axis.y == 0.0) || (axis.x == 1.0 && axis.y == 0.0)) {
      vector_t player_centroid = body_get_centroid(((pulley_param_t *)aux)->body);
      vector_t platform_centroid = body_get_centroid(coll);
      if (axis.x < 0.0) {
        double x_offset = (platform_centroid.x + body_get_width(coll)/2) - (player_centroid.x - body_get_width(((pulley_param_t *)aux)->body)/2);
        body_set_centroid(((pulley_param_t *)aux)->body, (vector_t) {player_centroid.x + x_offset + 1, player_centroid.y});
      }
      if (axis.x > 0.0) {
        double x_offset = (player_centroid.x + body_get_width(((pulley_param_t *)aux)->body)/2) - (platform_centroid.x - body_get_width(coll)/2);
        body_set_centroid(((pulley_param_t *)aux)->body, (vector_t) {player_centroid.x - x_offset - 1, player_centroid.y}); 
      }
      body_set_xvelocity(((pulley_param_t *)aux)->body, 0);
    }
    if (axis.x == 0.0 && axis.y == 1.0) {

      vector_t player_centroid = body_get_centroid(((pulley_param_t *)aux)->body);
      vector_t platform_centroid = body_get_centroid(coll);

      double mass1 = body_get_mass(((pulley_param_t *)aux)->body);

      if (mass1 != INFINITY && (platform_centroid.y - body_get_height(coll)/2) > (player_centroid.y + body_get_height(((pulley_param_t *)aux)->body)/2) - 1) {
        double y_offset = (platform_centroid.y - body_get_height(coll)/2) - (player_centroid.y + body_get_height(((pulley_param_t *)aux)->body)/2);
        body_set_centroid(((pulley_param_t *)aux)->body, (vector_t) {player_centroid.x, player_centroid.y + y_offset - 1});
      }
      body_set_yvelocity(((pulley_param_t *)aux)->body, 0.0);
      body_set_yvelocity(coll, 0.0);
      body_set_yvelocity(no_coll, 0.0);
      body_set_acceleration(((pulley_param_t *)aux)->body, VEC_ZERO);
      body_set_acceleration(coll, VEC_ZERO);
      body_set_acceleration(no_coll, VEC_ZERO);
      body_set_ground(coll, true);
    }
  }
}

void create_door_collision(scene_t *scene, body_t *door_red, body_t *player1,
                                           body_t *door_blue, body_t *player2)
{
  door_param_t *door = malloc(sizeof(door_param_t));
  door->check_door1 = door_red;
  door->player1 = player1;
  door->check_door2 = door_blue;
  door->player2 = player2;
  list_t *bodies = list_init(6, NULL);
  list_add(bodies, door_red);
  list_add(bodies, player1);
  list_add(bodies, door_blue);
  list_add(bodies, player2);
  scene_add_bodies_force_creator(scene, (force_creator_t)crt_door, door, bodies,
                                 (free_func_t)door_free);
}

void crt_fall(void *aux) {
  double y_grav = ((one_body_param_t *)aux)->constant * body_get_mass(((one_body_param_t *)aux)->body);
  vector_t gravity = {0, y_grav};
  body_add_force(((one_body_param_t *)aux)->body, vec_negate(gravity));
}

void crt_fan(void *aux) {
  list_t *l1 = body_get_shape(((two_body_param_t *)aux)->body1);
  list_t *l2 = body_get_shape(((two_body_param_t *)aux)->body2);

  collision_info_t collide = find_collision(l1, l2);
  bool collision = collide.collided;

  list_free(l1);
  list_free(l2);

  if (collision)
  {
    double net = ((two_body_param_t *)aux)->constant * body_get_mass(((two_body_param_t *)aux)->body1);
    body_add_impulse(((two_body_param_t *)aux)->body1, (vector_t){0, net});
  }
}

void crt_button(void *aux) {
  list_t *l1 = body_get_shape(((two_body_param_t *)aux)->body1);
  list_t *l2 = body_get_shape(((two_body_param_t *)aux)->body2);

  collision_info_t collide = find_collision(l1, l2);
  bool collision = collide.collided;

  list_free(l1);
  list_free(l2);

  body_fan(((two_body_param_t *)aux)->body1, collision);
}

void create_newtonian_gravity(scene_t *scene, double G, body_t *body1,
                              body_t *body2) {
  two_body_param_t *grav = malloc(sizeof(two_body_param_t));
  grav->constant = G;
  grav->body1 = body1;
  grav->body2 = body2;
  list_t *bodies = list_init(2, NULL);
  list_add(bodies, body1);
  list_add(bodies, body2);
  scene_add_bodies_force_creator(scene, (force_creator_t)crt_gravity, grav,
                                 bodies, (free_func_t)two_free);
}

void create_spring(scene_t *scene, double k, body_t *body1, body_t *body2) {
  two_body_param_t *spring = malloc(sizeof(two_body_param_t));
  spring->constant = k;
  spring->body1 = body1;
  spring->body2 = body2;
  list_t *bodies = list_init(2, NULL);
  list_add(bodies, body1);
  list_add(bodies, body2);
  scene_add_bodies_force_creator(scene, (force_creator_t)crt_spring, spring,
                                 bodies, (free_func_t)two_free);
}

void create_drag(scene_t *scene, double gamma, body_t *body) {
  one_body_param_t *drag = malloc(sizeof(one_body_param_t));
  drag->constant = gamma;
  drag->body = body;
  list_t *bodies = list_init(1, NULL);
  list_add(bodies, body);
  scene_add_bodies_force_creator(scene, (force_creator_t)crt_drag, drag, bodies,
                                 (free_func_t)one_free);
}

void create_collision(scene_t *scene, body_t *body1, body_t *body2,
                      collision_handler_t handler, void *aux,
                      free_func_t freer) {
  two_bodies_param_t *collision = malloc(sizeof(two_bodies_param_t));
  ((two_bodies_param_t *)collision)->body1 = body1;
  ((two_bodies_param_t *)collision)->body2 = body2;
  ((two_bodies_param_t *)collision)->handler = handler;
  ((two_bodies_param_t *)collision)->aux = aux;
  ((two_bodies_param_t *)collision)->freer = freer;
  list_t *bodies = list_init(2, NULL);
  list_add(bodies, body1);
  list_add(bodies, body2);
  scene_add_bodies_force_creator(scene, (force_creator_t)crt_collision,
                                 collision, bodies,
                                 (free_func_t)twos_free);
}

void create_plat_collision(scene_t *scene, double k, body_t *body1,
                                  body_t *body2) {
  two_body_param_t *collision = malloc(sizeof(two_bodies_param_t));
  collision->constant = k;
  collision->body1 = body1;
  collision->body2 = body2;
  list_t *bodies = list_init(2, NULL);
  list_add(bodies, body1);
  list_add(bodies, body2);
  scene_add_bodies_force_creator(scene, (force_creator_t)crt_plat,
                                 collision, bodies,
                                 (free_func_t)two_free);
}


void create_fall(scene_t *scene, double G, body_t *body) {
  one_body_param_t *net = malloc(sizeof(one_body_param_t));
  net->constant = G;
  net->body = body;
  list_t *bodies = list_init(1, NULL);
  list_add(bodies, body);
  scene_add_bodies_force_creator(scene, (force_creator_t)crt_fall, net, bodies,
                                 (free_func_t)one_free);
}

void create_fan(scene_t *scene, double k, body_t *body1, body_t *body2) {
  two_body_param_t *fan = malloc(sizeof(two_body_param_t));
  fan->constant = k;
  fan->body1 = body1;
  fan->body2 = body2;
  list_t *bodies = list_init(2, NULL);
  list_add(bodies, body1);
  list_add(bodies, body2);
  scene_add_bodies_force_creator(scene, (force_creator_t)crt_fan, fan, bodies,
                                 (free_func_t)two_free);
}

void create_button(scene_t *scene, body_t *body1, body_t *body2) {
  two_body_param_t *button = malloc(sizeof(two_body_param_t));
  button->body1 = body1;
  button->body2 = body2;
  list_t *bodies = list_init(2, NULL);
  list_add(bodies, body1);
  list_add(bodies, body2);
  scene_add_bodies_force_creator(scene, (force_creator_t)crt_button, button, bodies,
                                 (free_func_t)two_free);
}

void create_pulley_collision(scene_t *scene, body_t *body, double constant, body_t *pulley1, body_t *pulley2){
  pulley_param_t *pulley = malloc(sizeof(pulley_param_t));
  pulley->body = body;
  pulley->pulley1 = pulley1;
  pulley->pulley2 = pulley2;
  pulley->constant = constant;
  list_t *bodies = list_init(3, NULL);
  list_add(bodies, body);
  list_add(bodies, pulley1);
  list_add(bodies, pulley2);
  scene_add_bodies_force_creator(scene, (force_creator_t)crt_pulley, pulley, bodies,
                                 (free_func_t)pulley_free);
}

void destructive_handler(body_t *body1, body_t *body2, vector_t axis,
                         void *aux) {
  body_remove(body1);
  body_remove(body2);
}

void create_destructive_collision(scene_t *scene, body_t *body1,
                                  body_t *body2) {
  handle_param_t *collision = malloc(sizeof(handle_param_t));
  create_collision(scene, body1, body2,
                   (collision_handler_t)destructive_handler, collision,
                   (free_func_t)handle_free);
}

void physics_handler(body_t *body1, body_t *body2, vector_t axis, void *aux) {
  double mass1 = body_get_mass(body1);
  double mass2 = body_get_mass(body2);
  double masses = mass1 * mass2 / (mass1 + mass2);
  if (mass2 == INFINITY) {
    masses = mass1;
  }

  double constant = masses * (1.0 + ((handle_param_t *)aux)->constant);
  double components = vec_dot(body_get_velocity(body2), axis) -
                      vec_dot(body_get_velocity(body1), axis);
  body_add_impulse(body1, vec_multiply(constant * components, axis));

  if (get_typ((info_t *)body_get_info(body2)) == 1) {
    ((handle_param_t *)aux)->hits = ((handle_param_t *)aux)->hits + 1;
    rgb_color_t col = body_get_color(body2);
    body_set_color(
        body2, (rgb_color_t){.r = col.r / 2, .g = col.g / 2, .b = col.b / 2});
    if (((handle_param_t *)aux)->hits == 3) {
      body_remove(body2);
    }
  } else if (get_typ((info_t *)body_get_info(body2)) != 0) {
    body_add_impulse(body2, vec_multiply(-1 * constant * components, axis));
  }
}

void create_physics_collision(scene_t *scene, double constant, body_t *body1,
                              body_t *body2) {
  handle_param_t *collision = malloc(sizeof(handle_param_t));
  collision->hits = 0;
  collision->constant = constant;
  collision->collided = false;
  create_collision(scene, body1, body2, (collision_handler_t)physics_handler,
                   collision, (free_func_t)handle_free);
}

void disappear_handler(body_t *body1, body_t *body2, vector_t axis,
                         void *aux) {
  body_remove(body1);
}

void create_disappear_collision(scene_t *scene, body_t *body1,
                                  body_t *body2) {
  handle_param_t *collision = malloc(sizeof(handle_param_t));
  create_collision(scene, body1, body2,
                   (collision_handler_t)disappear_handler, collision,
                   (free_func_t)handle_free);
}

void exit_handler(body_t *body1, body_t *body2, vector_t axis,
                         void *aux) {
  body_lose(body1);
}

void create_exit_collision(scene_t *scene, body_t *body1,
                                  body_t *body2) {
  create_collision(scene, body1, body2, exit_handler, NULL, NULL);
}

void two_free(two_body_param_t *t) { free(t); }

void one_free(one_body_param_t *o) { free(o); }

void twos_free(two_bodies_param_t *ts) {
  if (ts->freer != NULL) {
    ts->freer(ts->aux);
  }
  free(ts);
}

void handle_free(handle_param_t *h) { free(h); }

void door_free(door_param_t *d) { free(d); }

void pulley_free(pulley_param_t *p) { free(p); }

