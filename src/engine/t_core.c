#include "tailored.h"

#include <stdlib.h>
#include <string.h>

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

void t_destroy_shader_program(unsigned int shader_program) {
  glDeleteProgram(shader_program);
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