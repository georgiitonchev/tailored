#include "t_sprite.h"

// GL loader
#include "../dep/include/glad/glad.h"

// MATH
#include "../dep/include/cglm/cglm.h"

extern t_global_state global_state;

static unsigned int sprite_quad_vao;
static unsigned int sprite_shader;

t_rect clip_areas[2];

static void init_shader() {
  sprite_shader = create_shader_program("./res/shaders/sprite_shader.vs",
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

void init_sprite_renderer() {
  init_shader();
  init_quad();

  clip_areas[0] = RECT_ZERO;
  clip_areas[1] = RECT_ZERO;
}

void draw_sprite(t_sprite* sprite, float x, float y, float width, float height, t_color color) {
  glUseProgram(sprite_shader);

  mat4 mat4_projection;
  glm_ortho(0, 640, 360, 0, 0.0, 1.0, mat4_projection);

  mat4 mat4_model;
  glm_mat4_identity(mat4_model);
  glm_translate(mat4_model,
                (vec3){x, y, -0.2f});
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

void draw_sprite_t(t_sprite *sprite, t_rect rect, t_color color) {
    draw_sprite(
      sprite,
      rect.x, rect.y, rect.width, rect.height, color);
}

void create_sprite(const char* path, t_sprite* sprite) {
  sprite->texture = load_texture(path);
  sprite->scale = (t_vec2){ 1, 1 };
  sprite->texture_slice = (t_vec4){ 0, 0, sprite->texture.size.x, sprite->texture.size.y };
  sprite->slice_borders = (t_vec4){ 0, 0, 0, 0 };

}
void delete_sprite(t_sprite* sprite)
{
    free_texture(&sprite->texture);
}

static void t_set_clip_area(const char* key, int x, int y, int width, int height) {
    glUniform4fv(glGetUniformLocation(sprite_shader, key), 1,
                  (vec4){
                      (x / global_state.window_size.x) * global_state.framebuffer_size.x,
                      ((global_state.window_size.y - y - height) / global_state.window_size.y) * global_state.framebuffer_size.y,
                      (width / global_state.window_size.x) * global_state.framebuffer_size.x,
                      (height / global_state.window_size.y) * global_state.framebuffer_size.y });
}

void t_begin_clip_area(int x, int y, int width, int height) {

  clip_areas[0] = (t_rect){ x, y, width, height };
  t_set_clip_area("u_clip_area", x, y, width, height);
}
void t_begin_clip_area_r(t_rect rect) {
    t_begin_clip_area(rect.x, rect.y, rect.width, rect.height);
}

void t_begin_clip_area_inverse(int index, int x, int y, int width, int height) {

  char uniform_name[22];
  sprintf(uniform_name, "u_clip_area_inverse_%d", index);

  t_set_clip_area(uniform_name, x, y, width, height);
}
void t_begin_clip_area_inverse_r(int index, t_rect rect) {
    t_begin_clip_area_inverse(index, rect.x, rect.y, rect.width, rect.height);
}

void t_end_clip_area() {
  clip_areas[0] = RECT_ZERO;

  glUniform4fv(glGetUniformLocation(sprite_shader, "u_clip_area"), 1,
                (vec4){ 0, 0, 0, 0 });
}

void t_end_clip_area_inverse(int index) {
  char uniform_name[22];
  sprintf(uniform_name, "u_clip_area_inverse_%d", index);

  glUniform4fv(glGetUniformLocation(sprite_shader, uniform_name), 1,
                (vec4){ 0, 0, 0, 0 });
}
