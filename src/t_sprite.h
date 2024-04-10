#include "tailored.h"

typedef struct t_sprite {
  t_texture *textue;
  t_vec4 slice_borders;
  t_vec2 scale;
} t_sprite;

void init_sprite_renderer();
void draw_sprite_t(t_sprite *sprite, t_transform transform);
void draw_sprite(t_sprite* sprite, t_vec2 position, t_vec2 size);
