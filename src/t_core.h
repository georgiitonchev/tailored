#pragma once
#define UNUSED(x) (void)(x)

#include <stdbool.h>

typedef enum {
  t_success = 0,
  t_error = 1
} t_result;

typedef struct t_vec2 {
  float x;
  float y;
} t_vec2;

typedef struct t_vec3 {
  float x;
  float y;
  float z;
} t_vec3;

typedef struct t_vec4 {
  float x;
  float y;
  float z;
  float w;

} t_vec4;

typedef struct t_color {
  float r;
  float g;
  float b;
  float a;

} t_color;

typedef struct t_rect {
  float x;
  float y;
  float width;
  float height;

} t_rect;

typedef struct t_quaternion {
  float x;
  float y;
  float z;
  float w;
} t_quaternion;

typedef struct t_vertex {
  t_vec3 position;
  t_vec3 normal;
  t_vec2 tex_coord;

} t_vertex;

typedef struct t_transform {
  t_vec3 position;
  t_quaternion rotation;
  t_vec3 size;
  t_vec3 scale;
} t_transform;

typedef struct t_texture {
  unsigned int id;
  t_vec2 size;
  unsigned int channels;
} t_texture;

typedef struct t_texture_data {
  unsigned char* data;
  unsigned int width;
  unsigned int height;
  unsigned int channels;
} t_texture_data;

typedef struct t_input_state {

  struct t_mouse_state {
    struct t_btn_state {
      bool is_pressed;
      bool is_down;
      bool is_released;
    } buttons[2];

  } mouse_state;

  struct t_keyboard_state {
    bool active;
  } keyboard_state;

} t_inpus_state;

typedef struct t_global_state {

  t_vec2 mouse_pos;
  t_inpus_state input_state;
  t_vec2 window_size;
  t_vec2 framebuffer_size;
  float delta_time;

} t_global_state;

// files
const char *read_file(const char *path);

// opengl
unsigned int create_shader_program(const char *vertex_shader_path,
                                   const char *fragment_shader_path);

void clear_color(t_color color);

t_texture_data load_texture_data(const char* path);
t_texture load_texture(const char *path);
void free_texture(t_texture* texture);


// COLORS
#define WHITE (t_color) { 255, 255, 255, 255 }
#define LIGHT_GRAY (t_color) { 163, 163, 163, 255 }
#define DARK_GRAY (t_color) { 81, 81, 81, 255 }
#define RED (t_color) { 255, 0, 0, 255 }
#define GREEN (t_color) { 0, 255, 0, 255 }
#define BLUE (t_color) { 0, 0, 255, 255 }

// VECTORS
#define VEC2_ZERO (t_vec2) { 0, 0 }
#define VEC2_ONE (t_vec2) { 1, 1 }
#define RECT_ZERO (t_rect) { 0, 0, 0, 0 }

t_vec2 vec2_sub(t_vec2 a, t_vec2 b);
float map(float value, float from_min, float from_max, float to_min, float to_max);
