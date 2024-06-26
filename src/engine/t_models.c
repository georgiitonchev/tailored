#include "tailored.h"

#define CGLTF_IMPLEMENTATION
#include "../../dep/include/cgltf/cgltf.h"

// STB
#include "../../dep/include/stb/stb_image.h"

// MATH
#include "../../dep/include/cglm/cglm.h"

static const char *get_directory_path(const char *path) {
  const char *last_slash = strrchr(path, '/');

  if (last_slash != NULL) {
    // Calculate the length of the substring
    size_t length = last_slash - path;

    // Allocate memory for the substring
    char *substring = malloc((length + 1) * sizeof(char));

    // Copy the substring
    strncpy(substring, path, length);
    substring[length] = '\0'; // Null-terminate the string

    return substring;
  }

  return NULL;
}

t_mesh process_mesh(cgltf_mesh *cgltf_mesh, const char *path) {

  t_mesh mesh = {0};

  for (cgltf_size pi = 0; pi < cgltf_mesh->primitives_count; pi++) {

    printf("Mesh primitives: %zu\n", cgltf_mesh->primitives_count);
    cgltf_primitive primitive = cgltf_mesh->primitives[pi];

    if (primitive.indices != NULL) {
      cgltf_accessor *data = primitive.indices;
      unsigned short *indices = malloc(data->count * sizeof(unsigned short));

      printf("Primitive indices: %zu\n", data->count);

      unsigned short *buffer =
          (unsigned short *)data->buffer_view->buffer->data +
          data->buffer_view->offset / sizeof(unsigned short) +
          data->offset / sizeof(unsigned short);

      int n = 0;
      for (unsigned int i = 0; i < data->count; i++) {
        indices[i] = buffer[n];
        n += (int)(data->stride / sizeof(unsigned short));
      }

      mesh.indices_count = data->count;
      mesh.indices = indices;

      printf("Primitive indices collected.\n");
    }

    if (primitive.material != NULL) {
      if (primitive.material->pbr_metallic_roughness.base_color_texture
              .texture != NULL) {

        printf("Loading base texture.\n.");
        const char *base_image_uri =
            primitive.material->pbr_metallic_roughness.base_color_texture
                .texture->image->uri;

        const char *directory_path = get_directory_path(path);

        unsigned long directory_path_length = strlen(directory_path);
        unsigned long base_image_uri_length = strlen(base_image_uri);
        unsigned long texture_path_length =
            directory_path_length + base_image_uri_length + 1;

        char *texture_path = malloc(texture_path_length * sizeof(char));

        strcpy(texture_path, directory_path);
        texture_path[directory_path_length] = '/';
        strcpy(texture_path + directory_path_length + 1, base_image_uri);
        texture_path[texture_path_length] = '\0';

        mesh.material.base_texture_uri = texture_path;
      } else {
        printf("Loading base color.\n");
        cgltf_float *base_color_factor =
            primitive.material->pbr_metallic_roughness.base_color_factor;

        mesh.material.base_color.x = base_color_factor[0];
        mesh.material.base_color.y = base_color_factor[1];
        mesh.material.base_color.z = base_color_factor[2];
      }
    }

    mesh.vertices_count = primitive.attributes[0].data->count;
    mesh.vertices = malloc(mesh.vertices_count * sizeof(t_vertex));

    for (cgltf_size ai = 0; ai < primitive.attributes_count; ai++) {

      cgltf_attribute attribute = primitive.attributes[ai];
      cgltf_accessor *data = attribute.data;

      if (attribute.type == cgltf_attribute_type_position) {

        int n = 0;
        float *buffer = (float *)data->buffer_view->buffer->data +
                        data->buffer_view->offset / sizeof(float) +
                        data->offset / sizeof(float);

        for (unsigned int k = 0; k < data->count; k++) {
          t_vec3 vertex = {0};
          vertex.x = buffer[n];
          vertex.y = buffer[n + 1];
          vertex.z = buffer[n + 2];

          mesh.vertices[k].position = vertex;

          n += (int)(data->stride / sizeof(float));
        }
      } else if (attribute.type == cgltf_attribute_type_normal) {

        int n = 0;
        float *buffer = (float *)data->buffer_view->buffer->data +
                        data->buffer_view->offset / sizeof(float) +
                        data->offset / sizeof(float);

        for (unsigned int k = 0; k < data->count; k++) {
          t_vec3 normal = {0};
          normal.x = buffer[n];
          normal.y = buffer[n + 1];
          normal.z = buffer[n + 2];

          mesh.vertices[k].normal = normal;

          n += (int)(data->stride / sizeof(float));
        }
      } else if (attribute.type == cgltf_attribute_type_texcoord) {

        int n = 0;
        float *buffer = (float *)data->buffer_view->buffer->data +
                        data->buffer_view->offset / sizeof(float) +
                        data->offset / sizeof(float);

        for (unsigned int k = 0; k < data->count; k++) {
          t_vec2 tex_coord = {0};
          tex_coord.x = buffer[n];
          tex_coord.y = buffer[n + 1];

          mesh.vertices[k].tex_coord = tex_coord;

          n += (int)(data->stride / sizeof(float));
        }
      }
    }
  }
  return mesh;
}

void process_gltf_file(const char *path, t_scene **scenes) {

  cgltf_options options = {0};
  cgltf_data *data = NULL;
  cgltf_result result = cgltf_parse_file(&options, path, &data);

  if (result == cgltf_result_success) {

    t_log_info(".gltf (%s) file loaded successfuly.", path);

    result = cgltf_load_buffers(&options, data, path);
    if (result == cgltf_result_success) {

      *scenes = malloc(sizeof(t_scene) * data->scenes_count);

      printf(".gltf (%s) buffers loaded successfuly.", path);
      printf("Scenes: %zu\n", data->scenes_count);

      for (cgltf_size i = 0; i < data->scenes_count; i++) {

        cgltf_scene cgltf_scene = data->scenes[i];
        scenes[i]->nodes_count = cgltf_scene.nodes_count;
        scenes[i]->nodes = malloc(sizeof(t_node) * cgltf_scene.nodes_count);

        printf("Nodes in scene %zu: %zu\n", i, cgltf_scene.nodes_count);

        for (cgltf_size ni = 0; ni < cgltf_scene.nodes_count; ni++) {
          cgltf_node *cgltf_node = cgltf_scene.nodes[ni];
          t_node node = {0};

          node.mesh = process_mesh(cgltf_node->mesh, path);
          setup_mesh(&node.mesh);

          if (cgltf_node->has_translation) {
            node.transform.position.x = cgltf_node->translation[0];
            node.transform.position.y = cgltf_node->translation[1];
            node.transform.position.z = cgltf_node->translation[2];
          }
          if (cgltf_node->has_rotation) {
            node.transform.rotation.x = cgltf_node->rotation[0];
            node.transform.rotation.y = cgltf_node->rotation[1];
            node.transform.rotation.z = cgltf_node->rotation[2];
            node.transform.rotation.w = cgltf_node->rotation[3];
          }
          if (cgltf_node->has_scale) {
            node.transform.scale.x = cgltf_node->scale[0];
            node.transform.scale.y = cgltf_node->scale[1];
            node.transform.scale.z = cgltf_node->scale[2];
          } else {
            node.transform.scale.x = 1;
            node.transform.scale.y = 1;
            node.transform.scale.z = 1;
          }

          scenes[i]->nodes[ni] = node;
        }
      }

      printf("meshes processed.\n");
    } else {
      t_log_error("Error loading .gltf buffers: %d", result);
    }

    cgltf_free(data);
  } else {
    t_log_error("Error loading .gltf file\n");
  }
}

void setup_mesh(t_mesh *mesh) {

  // generate arrays and buffers
  glGenVertexArrays(1, &mesh->gl_vertex_array);    // VAO
  glGenBuffers(1, &mesh->gl_array_buffer);         // VBO
  glGenBuffers(1, &mesh->gl_element_array_buffer); // EBO

  glBindVertexArray(mesh->gl_vertex_array);             // VAO
  glBindBuffer(GL_ARRAY_BUFFER, mesh->gl_array_buffer); // VBO

  // bind vertices data
  glBufferData(
      GL_ARRAY_BUFFER, mesh->vertices_count * sizeof(t_vertex), mesh->vertices,
      GL_STATIC_DRAW); // mesh->vertices might need to be something else

  if (mesh->indices_count > 0) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->gl_element_array_buffer);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER, mesh->indices_count * sizeof(unsigned short),
        mesh->indices,
        GL_STATIC_DRAW); // mesh->indices might need to be something else
  }

  // positions
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(t_vertex),
                        0); // may need to cast 0 to void*

  // normals
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, sizeof(t_vertex),
                        (void *)offsetof(t_vertex, normal));

  // tex coords
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_TRUE, sizeof(t_vertex),
                        (void *)offsetof(t_vertex, tex_coord));

  glBindVertexArray(0);

  if (mesh->material.base_texture_uri != NULL) {
    printf("Loading material base texture.\n");
    mesh->material.base_texture = t_load_texture(mesh->material.base_texture_uri).id;
  }
}

void draw_mesh(t_mesh *mesh, unsigned int shader_program) {
  if (mesh->material.base_texture != 0) {
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(shader_program, "u_texture_base"), 0);
    glBindTexture(GL_TEXTURE_2D, mesh->material.base_texture);
  } else {
    glUniform1i(glGetUniformLocation(shader_program, "u_use_base_color"), true);
    glUniform3fv(glGetUniformLocation(shader_program, "u_color_base"), 1,
                 (vec3){mesh->material.base_color.x,
                        mesh->material.base_color.y,
                        mesh->material.base_color.z});
  }

  glBindVertexArray(mesh->gl_vertex_array);

  if (mesh->indices_count > 0)
    glDrawElements(GL_TRIANGLES, mesh->indices_count, GL_UNSIGNED_SHORT, 0);
  else
    glDrawArrays(GL_TRIANGLES, 0, mesh->vertices_count);
}
