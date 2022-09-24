#include "polygon.h"
#include "list.h"
#include "stdlib.h"
#include "vector.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

double polygon_area(list_t *polygon) {
  double sum = 0;
  size_t size = list_size(polygon);
  for (size_t i = 0; i < size; i++) {
    sum += vec_cross(*(vector_t *)list_get(polygon, i),
                     *(vector_t *)list_get(polygon, (i + 1) % size));
  }
  return sum / 2.0;
}

vector_t polygon_centroid(list_t *polygon) {
  double Cx = 0;
  double Cy = 0;
  vector_t *curr;
  vector_t *next;
  double A = polygon_area(polygon);
  for (size_t i = 0; i < list_size(polygon) - 1; i++) {
    curr = (vector_t *)list_get(polygon, i);
    next = (vector_t *)list_get(polygon, i + 1);
    Cx += (curr->x + next->x) * (vec_cross(*curr, *next));
    Cy += (curr->y + next->y) * (vec_cross(*curr, *next));
  }
  curr = (vector_t *)list_get(polygon, list_size(polygon) - 1);
  next = (vector_t *)list_get(polygon, 0);
  Cx += (curr->x + next->x) * (vec_cross(*curr, *next));
  Cy += (curr->y + next->y) * (vec_cross(*curr, *next));

  vector_t v = {.x = Cx / (6 * A), .y = Cy / (6 * A)};
  return v;
}

void polygon_translate(list_t *polygon, vector_t translation) {
  for (size_t i = 0; i < list_size(polygon); i++) {
    vector_t *curr_vector = (vector_t *)list_get(polygon, i);
    *curr_vector = vec_add(translation, *curr_vector);
  }
}

void polygon_rotate(list_t *polygon, double angle, vector_t point) {
  for (size_t i = 0; i < list_size(polygon); i++) {
    vector_t *curr_vector = (vector_t *)list_get(polygon, i);
    curr_vector->x = curr_vector->x - point.x;
    curr_vector->y = curr_vector->y - point.y;
    *curr_vector = vec_rotate(*curr_vector, angle);
    curr_vector->x = curr_vector->x + point.x;
    curr_vector->y = curr_vector->y + point.y;
  }
}
