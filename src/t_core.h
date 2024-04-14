#pragma once

typedef struct t_vec2 {
  float x;
  float y;
} t_vec2;

typedef struct t_vec3 {
  float x;
  float y;
  float z;
} t_vec3;

typedef union t_vec4 {
  struct {
    float x;
    float y;
    float z;
    float w;
  };

  struct {
    float r;
    float g;
    float b;
    float a;
  };
 
} t_vec4;

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
} t_transform;

typedef struct t_texture {
  unsigned int id;
  t_vec2 size;
  unsigned int channels;
} t_texture;

// files
const char *read_file(const char *path);

// opengl
unsigned int create_shader_program(const char *vertex_shader_path,
                                   const char *fragment_shader_path);

t_texture *load_texture(const char *path);


#define WHITE (t_vec4) { 1, 1, 1, 1 }
#define RED (t_vec4) { 1, 0, 0, 1 }
#define GREEN (t_vec4) { 0, 1, 0, 1 }
#define BLUE (t_vec4) { 0, 0, 1, 1 }