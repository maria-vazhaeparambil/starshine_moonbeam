#include "list.h"
#include "polygon.h"
#include "sdl_wrapper.h"
#include "state.h"
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

const size_t PACMAN_POINTS = 306;
const size_t PELLET_POINTS = 360;
const double PACMAN_ANGLE = 305.0;
const double CIRC_ANGLE = 360.0;

const double ROTATION_CONV = M_PI / 180.0;

float get_rand_pos(size_t max) { return (float)rand() / (float)RAND_MAX * max; }

list_t *draw_pacman(size_t radius, vector_t centroid) {
  size_t n = PACMAN_POINTS - 1;
  list_t *new_points = list_init(n + 1, free);
  double shift = (CIRC_ANGLE - PACMAN_ANGLE) / 2.0;
  for (size_t i = 0; i < n; i++) {
    vector_t *out = malloc(sizeof(vector_t));
    out->x = radius * cos(ROTATION_CONV * (PACMAN_ANGLE / n * i + shift)) +
             centroid.x;
    out->y = radius * sin(ROTATION_CONV * (PACMAN_ANGLE / n * i + shift)) +
             centroid.y;
    list_add(new_points, out);
  }
  vector_t *in = malloc(sizeof(vector_t));
  *in = vec_add(VEC_ZERO, centroid);
  list_add(new_points, in);
  return new_points;
}

list_t *draw_pellet(size_t radius, vector_t centroid) {
  size_t n = PELLET_POINTS;
  list_t *new_points = list_init(n, free);
  for (size_t i = 0; i < n; i++) {
    vector_t *out = malloc(sizeof(vector_t));
    *out = (vector_t){
        .x = radius * cos(ROTATION_CONV * CIRC_ANGLE / n * i) + centroid.x,
        .y = radius * sin(ROTATION_CONV * CIRC_ANGLE / n * i) + centroid.y};
    list_add(new_points, out);
  }
  return new_points;
}
