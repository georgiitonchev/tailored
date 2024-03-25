#include "../dep/include/cgltf/cgltf.h"

typedef struct vec3 {
  float x;
  float y;
  float z;
} vec3;

typedef struct mesh {
  vec3 *vertices;
  unsigned int *indices;
} mesh;

typedef struct model {
  unsigned int meshes_count;
  mesh *meshes;
} model;

void free_mesh(mesh *mesh);
void free_model(model *model);

void process_node(cgltf_node *node);
void process_scene(cgltf_scene *scene);
mesh process_mesh(cgltf_mesh *mesh);
