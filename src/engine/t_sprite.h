#pragma once
#include "tailored.h"

typedef struct t_sprite {
  t_texture texture;
  t_vec4 slice_borders;
  t_vec2 scale;
  t_vec4 texture_slice;

} t_sprite;

void init_sprite_renderer();

void create_sprite(const char* path, t_sprite* sprite);
void delete_sprite(t_sprite* sprite);

void draw_sprite_t(t_sprite *sprite, t_rect rect, t_color color);
void draw_sprite(t_sprite* sprite, float x, float y, float width, float height, t_color color);

void t_begin_clip_area_inverse(int index, int x, int y, int width, int height);
void t_begin_clip_area_inverse_r(int index, t_rect rect);
void t_end_clip_area_inverse(int index);

void t_begin_scissor(int x, int y, int width, int height);
void t_end_scissor();
