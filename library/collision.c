#include "collision.h"
#include "body.h"
#include "scene.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

double overlap(vector_t proj1, vector_t proj2) {
  if (proj1.y > proj2.x && proj1.x < proj2.y) {
    return proj1.y - proj2.x;
  } else if (proj2.y > proj1.x && proj2.x < proj1.y) {
    return proj2.y - proj1.x;
  }
  return -1;
}

vector_t calculate_projection(vector_t perp, list_t *body) {
  double min = 0;
  double max = 0;
  for (size_t j = 0; j < list_size(body); j++) {
    double curr = vec_dot(*(vector_t *)(list_get(body, j)), perp);
    if (j == 0) {
      min = curr;
      max = curr;
    } else {
      if (curr < min)
        min = curr;
      if (curr > max)
        max = curr;
    }
  }
  vector_t projection = {.x = min, .y = max};
  return projection;
}

list_t *calculate_axes(list_t *shape) {
  list_t *axes = list_init(list_size(shape), free);
  for (size_t i = 0; i < list_size(shape); i++) {
    vector_t *p1 = (vector_t *)list_get(shape, i);
    vector_t *p2 = (vector_t *)list_get(shape, (i + 1) % list_size(shape));
    vector_t new_perp = vec_perpendicular(vec_subtract(*p1, *p2));
    vector_t *vp = malloc(sizeof(vector_t));
    double length = sqrt(new_perp.x * new_perp.x + new_perp.y * new_perp.y);
    vp->x = new_perp.x / length;
    vp->y = new_perp.y / length;
    list_add(axes, vp);
  }
  return axes;
}

collision_info_t find_collision(list_t *shape1, list_t *shape2) {
  bool collision = true;

  list_t *axis1 = calculate_axes(shape1);
  list_t *axis2 = calculate_axes(shape2);

  vector_t projection1;
  vector_t projection2;

  double min_overlap = INFINITY;
  vector_t min = VEC_ZERO;

  for (size_t i = 0; i < list_size(axis1);
       i++) {
    projection1 = calculate_projection(*(vector_t *)list_get(axis1, i), shape1);
    projection2 = calculate_projection(*(vector_t *)list_get(axis1, i), shape2);
    double ov = overlap(projection1, projection2);
    if (ov < 0) {
      collision = false;
      break;
    } else if (ov < min_overlap) {
      min_overlap = ov;
      min = *(vector_t *)list_get(axis1, i);
    }
  }
  if (collision) {
    for (size_t i = 0; i < list_size(axis2);
         i++) { // calculating projections for shape 1, checking for overlap
      projection1 =
          calculate_projection(*(vector_t *)list_get(axis2, i), shape1);
      projection2 =
          calculate_projection(*(vector_t *)list_get(axis2, i), shape2);
      double ov = overlap(projection1, projection2);
      if (ov < 0) {
        collision = false;
        break;
      } else if (ov < min_overlap) {
        min_overlap = ov;
        min = *(vector_t *)list_get(axis2, i);
      }
    }
  }
  collision_info_t collide = {.collided = false};
  if (collision) {
    collide = (collision_info_t){.collided = true, .axis = min};
  }

  list_free(axis1);
  list_free(axis2);
  return collide;
}
