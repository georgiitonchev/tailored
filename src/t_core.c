#include "t_core.h"

#include "../dep/include/glad/glad.h"
#include <stdlib.h>
#include <string.h>

// STB
#define STB_IMAGE_IMPLEMENTATION
#include "../dep/include/stb/stb_image.h"
#define STB_DS_IMPLEMENTATION
#include "../dep/include/stb/stb_ds.h"

// MATH
#include "../dep/include/cglm/cglm.h"

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

const char *read_file(const char *path) {
  char *text_buffer;

  FILE *file_pointer = fopen(path, "rb");
  size_t file_size;

  if (file_pointer == NULL) {
    perror(path);
    exit(EXIT_FAILURE);
  }

  fseek(file_pointer, 0, SEEK_END);
  file_size = ftell(file_pointer);
  rewind(file_pointer);

  text_buffer = (char *)malloc((file_size + 1) * sizeof(char));

  // Read file contents into the buffer
  fread(text_buffer, sizeof(char), file_size, file_pointer);

  // Add null terminator at the end to make it a valid C string
  text_buffer[file_size] = '\0';

  fclose(file_pointer);

  return text_buffer;
}

unsigned int create_shader_program(const char *vertex_shader_path,
                                   const char *fragment_shader_path) {
  const char *vertex_shader_source = read_file(vertex_shader_path);

  unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
  glCompileShader(vertex_shader);

  free((char *)vertex_shader_source);

  // check for shader compile errors
  int success;
  char info_log[1024];
  glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertex_shader, 1024, NULL, info_log);
    printf("%s", info_log);
  } else
    printf("vertex shader compiled successfuly.\n");

  const char *fragment_shader_source = read_file(fragment_shader_path);

  // fragment shader
  unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
  glCompileShader(fragment_shader);

  free((char *)fragment_shader_source);

  // check for shader compile errors
  glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragment_shader, 1024, NULL, info_log);
    printf("%s", info_log);
  } else
    printf("fragment shader compiled successfuly.\n");
  // link shaders
  unsigned int shader_program = glCreateProgram();
  glAttachShader(shader_program, vertex_shader);
  glAttachShader(shader_program, fragment_shader);
  glLinkProgram(shader_program);
  // check for linking errors
  glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(shader_program, 1024, NULL, info_log);
    printf("%s", info_log);
  } else
    printf("program linked successfuly.\n");

  printf("shader program id: %d\n", shader_program);
  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);
  return shader_program;
}

t_texture *load_texture(const char *texture_path) {
  int width, height, channels;
  unsigned char *data = stbi_load(texture_path, &width, &height, &channels, 0);

  GLenum texture_format = 0;
  if (channels == 1)
    texture_format = GL_RED;
  else if (channels == 3)
    texture_format = GL_RGB;
  else if (channels == 4)
    texture_format = GL_RGBA;

  unsigned int texture_id;
  glGenTextures(1, &texture_id);
  glBindTexture(GL_TEXTURE_2D, texture_id);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glTexImage2D(GL_TEXTURE_2D, 0, texture_format, width, height, 0,
               texture_format, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, 0);

  stbi_image_free(data);

  t_texture *texture = malloc(sizeof(t_texture));
  texture->id = texture_id;
  texture->size.x = width;
  texture->size.y = width;
  texture->channels = channels;
  
  return texture;
}
