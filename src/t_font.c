#include <string.h>
#include "t_font.h"
#include "t_core.h"

// GL loader
#include "../dep/include/glad/glad.h"

// MATH
#include "../dep/include/cglm/cglm.h"

static unsigned int font_quad_vao;
static unsigned int font_shader;
static t_texture font_texture;

static void init_quad() {
  // configure VAO/VBO
  unsigned int vertex_buffer_object;

  float vertices[] = {
      // pos      // tex
      0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,

      0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f};

  glGenVertexArrays(1, &font_quad_vao);
  glGenBuffers(1, &vertex_buffer_object);

  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glBindVertexArray(font_quad_vao);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

static void init_shader() {
  font_shader = create_shader_program("./res/shaders/font_shader.vs",
                                        "./res/shaders/font_shader.fs");
}

static t_vec4 get_character(char character) {
  int columns = 16;

  int row = ((int)character) / columns;
  int column = ((int)character % columns);

  int left = column * 32;
  int top = row * 32;

  t_vec4 slice = {left, top, 32, 32};
  return slice;
}

static void draw_texture_slice(t_vec4 texture_slice, t_vec2 position, t_vec2 size, t_color color) {
  glUseProgram(font_shader);

  mat4 mat4_projection;
  glm_ortho(0, 640, 360, 0, 0.0, 1.0, mat4_projection);

  mat4 mat4_model;
  glm_mat4_identity(mat4_model);
  glm_translate(mat4_model,
                (vec3){position.x, position.y, 0.0f});
  glm_scale(mat4_model, (vec3){size.x, size.y, 1.0f});

  glUniformMatrix4fv(glGetUniformLocation(font_shader, "u_mat4_projection"),
                      1, GL_FALSE, (float *)mat4_projection);

  glUniformMatrix4fv(glGetUniformLocation(font_shader, "u_mat4_model"), 1,
                      GL_FALSE, (float *)mat4_model);

  glUniform4fv(glGetUniformLocation(font_shader, "u_color"), 1,
                (vec4){color.r, color.g, color.b, color.a});

  glUniform4fv(glGetUniformLocation(font_shader, "u_texture_slice"), 1, 
                (vec4){texture_slice.x, texture_slice.y, 
                        texture_slice.z, texture_slice.w});

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, font_texture.id);

  glBindVertexArray(font_quad_vao);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindVertexArray(0);
}

void init_font_renderer() {
  init_quad();
  init_shader();

  font_texture = load_texture("./res/textures/font.png");
}

void terminate_font_renderer() {
    free_texture(&font_texture);
}

void draw_text(const char* text, t_vec2 position, int size , t_color color) {
  for (int i = 0; i < (int) strlen(text); i++) {
      draw_texture_slice(get_character(text[i]), (t_vec2){ position.x + i * size, position.y }, (t_vec2){ size, size }, color);
  }
}

t_vec2 measure_text_size(const char* text, int size) {
    return (t_vec2) { size * strlen(text), size };
}
