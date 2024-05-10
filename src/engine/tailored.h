#pragma once
#define UNUSED(x) (void)(x)

#include <stdbool.h>

// SCTRUCTURE DEFINITIONS
typedef enum t_result {
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

    t_vec2 position;
  } mouse_state;

  struct t_keyboard_state {
    bool active;
  } keyboard_state;

} t_input_state;

// FILE FUNCTIONS
const char *t_read_file(const char *path, long* file_size);

// SHADER FUNCTIONS
unsigned int t_create_shader_program(const char *vertex_shader_path, const char *fragment_shader_path);

// COLOR FUNCTIONS
void t_clear_color(t_color color);

// TEXTURE FUNCTIONS
t_texture_data t_load_texture_data(const char* path);
t_texture t_load_texture(const char *path);
t_texture t_load_texture_from_data(const t_texture_data* texture_data);
void t_free_texture(t_texture* texture);

// MATH FUNCTIONS
t_vec2 t_vec2_sub(t_vec2 a, t_vec2 b);
float t_map(float value, float from_min, float from_max, float to_min, float to_max);

// ENGINE FUNCTIONS
int t_begin(int window_width, int window_height, const char* title);
bool t_loop();
void t_loop_end();
void t_end();

void t_draw_scene();

// AUDIO 
#include "./extern/miniaudio.h"
void t_play_audio(const char* path);
ma_result t_init_sound(const char* path, ma_sound* sound);
void t_fade_out_sound(ma_sound* sound, int time);
void t_start_sound(ma_sound* sound);
void t_set_master_volume(float value);

// CORE FUNCTIONS
void t_set_cursor(const char* path, int xhot, int yhot);
float t_delta_time();
t_vec2 t_window_size();
t_vec2 t_framebuffer_size();

// STRUCTURE VALUES
#define WHITE (t_color) { 255, 255, 255, 255 }
#define LIGHT_GRAY (t_color) { 163, 163, 163, 255 }
#define DARK_GRAY (t_color) { 81, 81, 81, 255 }
#define BLACK (t_color) { 0, 0, 0, 255 }
#define RED (t_color) { 255, 0, 0, 255 }
#define GREEN (t_color) { 0, 255, 0, 255 }
#define BLUE (t_color) { 0, 0, 255, 255 }

#define VEC2_ZERO (t_vec2) { 0, 0 }
#define VEC2_ONE (t_vec2) { 1, 1 }

#define VEC3_ZERO (t_vec3) { 0, 0, 0 }
#define VEC3_ONE (t_vec3) { 1, 1, 1 }

#define VEC4_ZERO (t_vec4) { 0, 0, 0, 0 }
#define VEC4_ONE (t_vec4) { 1, 1, 1, 1 }

#define RECT_ZERO (t_rect) { 0, 0, 0, 0 }