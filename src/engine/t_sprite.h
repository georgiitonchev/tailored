#pragma once
#include "tailored.h"

typedef struct t_sprite {
  t_texture texture;
  t_texture_data texture_data;

  t_vec4 slice_borders;
  t_vec2 scale;
  t_vec4 texture_slice;

} t_sprite;

void t_init_sprite_renderer();

void t_load_texture_data_s(t_sprite* sprite, const char* texture_path);
void t_init_sprite(t_sprite* sprite);
void t_deinit_sprite(t_sprite* sprite);

void t_draw_sprite(t_sprite* sprite, float x, float y, float width, float height, t_color color);
void t_draw_sprite_r(t_sprite *sprite, t_rect rect, t_color color);

void t_begin_scissor(int x, int y, int width, int height);
void t_end_scissor();
