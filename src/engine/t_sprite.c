#include "tailored.h"

// GL loader
#include "../../dep/include/glad/glad.h"

// MATH
#include "../../dep/include/cglm/cglm.h"

extern t_input_state input_state;

static unsigned int sprite_quad_vao;
static unsigned int sprite_shader;

static void init_shader() {
  sprite_shader = t_create_shader_program("./res/shaders/sprite_shader.vs",
                                        "./res/shaders/sprite_shader.fs");
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

void t_init_sprite_renderer() {
  init_shader();
  init_quad();
}

static void s_draw_sprite(t_sprite* sprite, float x, float y, float rot, float width, float height, t_color color) {
  if (t_clip_area().width + t_clip_area().height > 0) {
        if (!does_rect_overlap_rect((t_rect){x, y, width, height}, t_clip_area()))
        return;
  }

  glUseProgram(sprite_shader);

  mat4 mat4_projection;
  t_vec2 window_size = t_window_size();
  glm_ortho(0, window_size.x, window_size.y, 0, 0.0, 1.0, mat4_projection);

  mat4 mat4_model;
  glm_mat4_identity(mat4_model);
  glm_translate(mat4_model, (vec3){x, y, -0.2f});
  glm_rotate(mat4_model, rot, (vec3) {0, 0, 1});
  glm_scale(mat4_model, (vec3){width, height, 1.0f});

  glUniformMatrix4fv(glGetUniformLocation(sprite_shader, "u_mat4_projection"),
                      1, GL_FALSE, (float *)mat4_projection);

  glUniformMatrix4fv(glGetUniformLocation(sprite_shader, "u_mat4_model"), 1,
                      GL_FALSE, (float *)mat4_model);

  glUniform4fv(glGetUniformLocation(sprite_shader, "u_color"), 1,
                (vec4){color.r / 255.0, color.g / 255.0, color.b / 255.0, color.a / 255.0});

  glUniform4fv(glGetUniformLocation(sprite_shader, "u_slice_borders"), 1,
                (vec4){sprite->slice_borders.x, sprite->slice_borders.y,
                      sprite->slice_borders.z, sprite->slice_borders.w});

  glUniform4fv(glGetUniformLocation(sprite_shader, "u_texture_slice"), 1,
                (vec4){sprite->texture_slice.x, sprite->texture_slice.y,
                        sprite->texture_slice.z, sprite->texture_slice.w});

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, sprite->texture.id);

  glBindVertexArray(sprite_quad_vao);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindVertexArray(0);
}

void t_draw_sprite(t_sprite* sprite, float x, float y, float width, float height, t_color color) {
  s_draw_sprite(sprite, x, y, 0, width, height, color);
}

void t_draw_sprite_r(t_sprite *sprite, t_rect rect, t_color color) {
    t_draw_sprite(
      sprite,
      rect.x, rect.y, rect.width, rect.height, color);
}

void t_draw_sprite_rot(t_sprite* sprite, float x, float y, float rot, float width, float height, t_color color) {
  s_draw_sprite(sprite, x, y, rot, width, height, color);
}

void t_load_texture_data_s(t_sprite* sprite, const char* texture_path) {

  t_texture_data texture_data = t_load_texture_data(texture_path);
  sprite->texture_data = texture_data;
}

void t_load_texture_s(t_sprite* sprite) {

  t_texture texture = t_load_texture_from_data(&sprite->texture_data);
  sprite->texture = texture;
}

void t_init_sprite(t_sprite* sprite) {

  t_load_texture_s(sprite);

  sprite->scale = (t_vec2){ 1, 1 };
  sprite->texture_slice = (t_vec4){ 0, 0, sprite->texture_data.width, sprite->texture_data.height };
  sprite->slice_borders = (t_vec4){ 0, 0, 0, 0 };
}

void t_deinit_sprite(t_sprite* sprite) {
  t_free_texture(&sprite->texture);
}
