#include "tailored.h"

#include <stdlib.h>
#include <string.h>

#include "../../dep/include/glad/glad.h"
// STB
#define STB_IMAGE_IMPLEMENTATION
#include "../../dep/include/stb/stb_image.h"
// MATH
#include "../../dep/include/cglm/cglm.h"

const char *t_read_file(const char *path, long* file_size) {
  char *text_buffer;

  FILE *file_pointer = fopen(path, "rb");

  if (file_pointer == NULL) {
    perror(path);
    exit(EXIT_FAILURE);
  }

  fseek(file_pointer, 0, SEEK_END);
  *file_size = ftell(file_pointer);
  rewind(file_pointer);

  text_buffer = (char *)malloc((*file_size + 1) * sizeof(char));

  // Read file contents into the buffer
  fread(text_buffer, sizeof(char), *file_size, file_pointer);

  // Add null terminator at the end to make it a valid C string
  text_buffer[*file_size] = '\0';

  fclose(file_pointer);

  return text_buffer;
}

static GLuint compile_shader(const char* shader_path, unsigned int shader_type) {

  long file_size;
  const char* shader_source = t_read_file(shader_path, &file_size);

  GLuint shader_id = glCreateShader(shader_type);
  glShaderSource(shader_id, 1, &shader_source, NULL);
  glCompileShader(shader_id);

  free((void *)shader_source);

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

GLuint t_create_shader_program(const char *vertex_shader_path, const char *fragment_shader_path) {

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

t_texture_data t_load_texture_data(const char* path) {
  int width, height, channels;
  unsigned char *data = stbi_load(path, &width, &height, &channels, 0);

  if (data == NULL) {
    printf("error loading texture: %s\n", path);
  }

  t_texture_data texture_data = {
    .data = data,
    .channels = channels,
    .width = width,
    .height = height
  };

  return texture_data;
}

t_texture t_load_texture_from_data(const t_texture_data* texture_data) {
  
  GLenum texture_format = 0;
  if (texture_data->channels == 1)
    texture_format = GL_RED;
  else if (texture_data->channels == 3)
    texture_format = GL_RGB;
  else if (texture_data->channels == 4)
    texture_format = GL_RGBA;

  unsigned int texture_id;    
  
  glGenTextures(1, &texture_id);
  glBindTexture(GL_TEXTURE_2D, texture_id);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  // GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_ONE };
  // glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);

  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glTexImage2D(GL_TEXTURE_2D, 0, texture_format, texture_data->width, texture_data->height, 0,
              texture_format, GL_UNSIGNED_BYTE, texture_data->data);


  glGenerateMipmap(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, 0);

  t_texture texture;
  texture.id = texture_id;
  texture.size.x = texture_data->width;
  texture.size.y = texture_data->height;
  texture.channels = texture_data->channels;

  return texture;
}

t_texture t_load_texture(const char *texture_path) {

  t_texture_data texture_data = t_load_texture_data(texture_path);

  GLenum texture_format = 0;
  if (texture_data.channels == 1)
    texture_format = GL_RED;
  else if (texture_data.channels == 3)
    texture_format = GL_RGB;
  else if (texture_data.channels == 4)
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

  glTexImage2D(GL_TEXTURE_2D, 0, texture_format, texture_data.width, texture_data.height, 0,
               texture_format, GL_UNSIGNED_BYTE, texture_data.data);


  //glGenerateMipmap(GL_TEXTURE_2D);

  glBindTexture(GL_TEXTURE_2D, 0);

  stbi_image_free(texture_data.data);

  t_texture texture;// = malloc(sizeof(t_texture));
      texture.id = texture_id;
      texture.size.x = texture_data.width;
      texture.size.y = texture_data.height;
      texture.channels = texture_data.channels;

  return texture;
}

void t_free_texture(t_texture* texture) 
{
    glDeleteTextures(1, &texture->id);
}

void t_clear_color(t_color color) {
    glClearColor(color.r / 255, color.g / 255, color.b / 255, color.a);
}

t_vec2 t_vec2_sub(t_vec2 a, t_vec2 b) {
  return (t_vec2) { a.x - b.x, a.y - b.y };
}

float t_map(float value, float from_min, float from_max, float to_min, float to_max) {
    return (value - from_min) / (from_max - from_min) * (to_max - to_min) + to_min;
}