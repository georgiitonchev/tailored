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

static GLuint compile_shader(const char* shader_path, unsigned int shader_type) {

  const char* shader_source = read_file(shader_path);

  GLuint shader_id = glCreateShader(shader_type);
  glShaderSource(shader_id, 1, &shader_source, NULL);
  glCompileShader(shader_id);

  free(shader_source);

  GLint compile_status;
  glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compile_status);

  if (compile_status == GL_FALSE) {

    GLsizei info_log_length;
    glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &info_log_length);

    char* info_log = (char*)alloca(info_log_length * sizeof(char));
    glGetShaderInfoLog(shader_id, info_log_length * sizeof(char), &info_log_length, info_log);

    printf("Failed to compile shader: %s", info_log);

    glDeleteShader(shader_id);
    return 0;
  }

  return shader_id;
}

GLuint create_shader_program(const char *vertex_shader_path,
                                   const char *fragment_shader_path) {

  GLuint vertex_shader = compile_shader(vertex_shader_path, GL_VERTEX_SHADER);
  GLuint fragment_shader = compile_shader(fragment_shader_path, GL_FRAGMENT_SHADER);

  // link shaders
  GLuint shader_program = glCreateProgram();
  glAttachShader(shader_program, vertex_shader);
  glAttachShader(shader_program, fragment_shader);
  glLinkProgram(shader_program);

  GLint link_status;
  glGetProgramiv(shader_program, GL_LINK_STATUS, &link_status);

  if (link_status == GL_FALSE) {

    GLsizei info_log_length;
    glGetProgramiv(shader_program, GL_INFO_LOG_LENGTH, &info_log_length);

    char* info_log = (char*)alloca(info_log_length * sizeof(char));
    glGetProgramInfoLog(shader_program, info_log_length * sizeof(char), &info_log_length, info_log);

    printf("Failed to compile program: %s", info_log);

    glDeleteProgram(shader_program);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    return 0;
  }

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
