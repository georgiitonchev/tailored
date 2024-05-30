#include "tailored.h"
#include <stdbool.h>

void t_init_shapes_renderer();

bool is_point_in_rect(t_vec2 point, t_rect rect);
bool is_rect_in_rect(t_rect this, t_rect that);
bool is_rect_zero(t_rect rect);
bool does_rect_overlap_rect(t_rect this, t_rect that);

void draw_rect(int x, int y, int width, int height, t_color color);
void draw_rect_lines(int x, int y, int width, int height, t_color color);

void draw_line(int x_from, int y_from, int x_to, int y_to, float width, t_color color);
