#pragma once

#include "../../dep/include/cgltf/cgltf.h"

#include "tailored.h"

typedef struct t_material {
  const char *base_texture_uri;
  t_vec3 base_color;

  unsigned int base_texture;
} t_material;

typedef struct t_mesh {
  unsigned int vertices_count;
  t_vertex *vertices;

  unsigned int indices_count;
  unsigned short *indices;

  t_material material;

  unsigned int gl_vertex_array;
  unsigned int gl_array_buffer;
  unsigned int gl_element_array_buffer;
} t_mesh;

typedef struct t_node {
  t_transform transform;
  t_mesh mesh;
} t_node;

typedef struct t_scene {
  t_node *nodes;
  unsigned int nodes_count;
} t_scene;


void free_mesh(t_mesh *mesh);
void free_scene(t_scene *scene);

//gltf
void process_node(cgltf_node *node);
void process_scene(cgltf_scene *scene);
t_mesh process_mesh(cgltf_mesh *mesh, const char *path);
void process_gltf_file(const char *path, t_scene **scenes);

void setup_mesh(t_mesh *mesh);
void draw_mesh(t_mesh *mesh, unsigned int shader_program);
