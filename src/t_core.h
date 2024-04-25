#pragma once
#define UNUSED(x) (void)(x)

#include <stdbool.h>

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
} t_global_state;

// files
const char *read_file(const char *path);

// opengl
unsigned int create_shader_program(const char *vertex_shader_path,
                                   const char *fragment_shader_path);

t_texture *load_texture(const char *path);


#define WHITE (t_color) { 1, 1, 1, 1 }
#define LIGHT_GRAY (t_color) { .64f, .64f, .64f, 1 }
#define DARK_GRAY (t_color) { .32f, .32f, .32f, 1 }
#define RED (t_color) { 1, 0, 0, 1 }
#define GREEN (t_color) { 0, 1, 0, 1 }
#define BLUE (t_color) { 0, 0, 1, 1 }
