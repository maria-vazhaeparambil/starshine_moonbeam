#include "body.h"
#include "forces.h"
#include "scene.h"
#include "sdl_wrapper.h"

const vector_t WINDOW = (vector_t){.x = 1000, .y = 500};
const size_t DOT_MASS = 1;
const size_t INVISIBLE_MASS = INFINITY;
const size_t NUM_CIRCLES = 50;
const size_t RADIUS = 1000 / (2 * NUM_CIRCLES);
const size_t CIRCLE_PTS = 360;
const double K = 0.2;
const double GAMMA = 0.05;
const double COLOR_FREQ = 0.25;
const size_t COLOR_CONVERT = 255;

const double CIRC_ANG = 360.0;
const double ROTAT_CONV = M_PI / 180.0;

typedef struct state {
  scene_t *scene;
} state_t;

list_t *draw_dot(size_t radius, vector_t centroid) {
  size_t n = CIRCLE_PTS;
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

void generate_dot(state_t *s, vector_t centroid, double mass,
                  rgb_color_t color) {
  body_t *b = body_init(draw_dot(RADIUS, centroid), mass, color);
  scene_add_body(s->scene, b);
}

double new_color_value(double adj_index) {
  return (sin(adj_index) * 127 + 128) / COLOR_CONVERT;
}

rgb_color_t generate_color(size_t index) {
  return (rgb_color_t){new_color_value(COLOR_FREQ * index),
                       new_color_value(COLOR_FREQ * index + 2),
                       new_color_value(COLOR_FREQ * index + 4)};
}

state_t *emscripten_init() {
  const double WAVE_AMP = 150;
  const double WAVE_FREQ = 0.05;
  const double WAVE_CENTER = 250;
  vector_t min = (vector_t){.x = 0, .y = 0};
  vector_t max = WINDOW;
  sdl_init(min, max);
  state_t *state = malloc(sizeof(state_t));
  state->scene = scene_init();
  for (size_t i = 0; i < NUM_CIRCLES; i++) {
    // invisible white dot
    generate_dot(state, (vector_t){.x = RADIUS + (2 * RADIUS * i), .y = 250},
                 INVISIBLE_MASS, (rgb_color_t){.r = 1, .g = 1, .b = 1});
    // colored, moving dot
    generate_dot(state,
                 (vector_t){.x = RADIUS + (2 * RADIUS * i),
                            .y = WAVE_AMP * cos(WAVE_FREQ * i) + WAVE_CENTER},
                 DOT_MASS, generate_color(i));
    create_spring(state->scene, K, scene_get_body(state->scene, 2 * i + 1),
                  scene_get_body(state->scene, 2 * i));
    create_drag(state->scene, GAMMA, scene_get_body(state->scene, 2 * i + 1));
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
