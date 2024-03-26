#include "tailored.h"

#include "../dep/include/glad/glad.h"
#include <stdlib.h>

#define CGLTF_IMPLEMENTATION
#include "../dep/include/cgltf/cgltf.h"

// STB
#define STB_IMAGE_IMPLEMENTATION
#include "../dep/include/stb/stb_image.h"
#define STB_DS_IMPLEMENTATION
#include "../dep/include/stb/stb_ds.h"

unsigned int load_texture(const char *texture_path) {
  int width, height, channels;
  unsigned char *data = stbi_load(texture_path, &width, &height, &channels, 0);

  GLenum texture_format = 0;
  if (channels == 1)
    texture_format = GL_RED;
  else if (channels == 3)
    texture_format = GL_RGB;
  else if (channels == 4)
    texture_format = GL_RGBA;

  unsigned int texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, texture_format, width, height, 0,
               texture_format, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, 0);

  stbi_image_free(data);

  return texture;
}

const char *get_directory_path(const char *path) {
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
    cgltf_primitive primitive = cgltf_mesh->primitives[pi];

    if (primitive.indices != NULL) {
      cgltf_accessor *data = primitive.indices;
      unsigned short *indices = malloc(data->count * sizeof(unsigned short));

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
    }

    if (primitive.material != NULL) {
      const char *base_image_uri = primitive.material->pbr_metallic_roughness
                                       .base_color_texture.texture->image->uri;

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

void process_gltf_file(const char *path, t_model *model) {

  cgltf_options options = {0};
  cgltf_data *data = NULL;
  cgltf_result result = cgltf_parse_file(&options, path, &data);

  if (result == cgltf_result_success) {

    printf(".gltf file loaded successfuly\n");

    result = cgltf_load_buffers(&options, data, path);
    if (result == cgltf_result_success) {

      printf(".gltf buffers loaded successfuly\n");
      for (cgltf_size i = 0; i < data->meshes_count; i++) {
        model->meshes = malloc(sizeof(t_mesh) * data->meshes_count);
        model->meshes[i] = process_mesh(&data->meshes[i], path);
      }

      printf("meshes processed.\n");
    } else {
      printf("error loading .gltf buffers\n");
    }
    // process_scene(data->scene);

    cgltf_free(data);
    printf("free data.\n");
  } else {
    printf("error loading .gltf file\n");
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

  mesh->material.base_texture = load_texture(mesh->material.base_texture_uri);
}

void draw_mesh(t_mesh *mesh, unsigned int shader_program) {
  glUseProgram(shader_program);

  glActiveTexture(GL_TEXTURE0);
  glUniform1i(glGetUniformLocation(shader_program, "u_texture_base"), 0);
  glBindTexture(GL_TEXTURE_2D, mesh->material.base_texture);

  glBindVertexArray(mesh->gl_vertex_array);

  if (mesh->indices_count > 0)
    glDrawElements(GL_TRIANGLES, mesh->indices_count, GL_UNSIGNED_SHORT, 0);
  else
    glDrawArrays(GL_TRIANGLES, 0, mesh->vertices_count);
}
