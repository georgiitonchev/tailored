#include "t_sprite.h"

// GL loader
#include "../dep/include/glad/glad.h"

// MATH
#include "../dep/include/cglm/cglm.h"

static unsigned int sprite_quad_vao;
static unsigned int sprite_shader;

static void init_shader() {
  sprite_shader = create_shader_program("./res/shaders/shader.vs",
                                        "./res/shaders/shader.fs");
}

static void init_quad() {
  // configure VAO/VBO
  unsigned int vertex_buffer_object;

  float vertices[] = {
      // pos      // tex
      0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,

      0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f};

  glGenVertexArrays(1, &sprite_quad_vao);
  glGenBuffers(1, &vertex_buffer_object);

  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glBindVertexArray(sprite_quad_vao);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void init_sprite_renderer() {
  init_shader();
  init_quad();
}

void draw_sprite(t_sprite *sprite, t_transform transform) {

  glUseProgram(sprite_shader);

  mat4 mat4_projection;
  glm_ortho(0, 640, 360, 0, 0.0, 1.0, mat4_projection);

  mat4 mat4_model;
  glm_mat4_identity(mat4_model);
  glm_translate(mat4_model,
                (vec3){transform.position.x, transform.position.y, 0.0f});
  glm_scale(mat4_model, (vec3){transform.size.x, transform.size.y, 1.0f});

  glUniformMatrix4fv(glGetUniformLocation(sprite_shader, "u_mat4_projection"),
                     1, GL_FALSE, (float *)mat4_projection);

  glUniformMatrix4fv(glGetUniformLocation(sprite_shader, "u_mat4_model"), 1,
                     GL_FALSE, (float *)mat4_model);

  glUniform4fv(glGetUniformLocation(sprite_shader, "u_color"), 1,
               (vec4){1, 1, 1, 1.0f});

  glUniform4fv(glGetUniformLocation(sprite_shader, "u_slice_borders"), 1,
               (vec4){sprite->slice_borders.x, sprite->slice_borders.y,
                      sprite->slice_borders.z, sprite->slice_borders.w});

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, sprite->textue->id);

  glBindVertexArray(sprite_quad_vao);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindVertexArray(0);
}
