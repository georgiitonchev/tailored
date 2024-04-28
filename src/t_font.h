#pragma once
#include "t_core.h"

void init_font_renderer();
void terminate_font_renderer();
t_vec4 get_character(char character);
void draw_text(const char* text, t_vec2 position, int size, t_color color);

t_vec2 measure_text_size(const char* text, int size);