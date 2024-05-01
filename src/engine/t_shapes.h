#include "tailored.h"
#include <stdbool.h>

bool is_point_in_rect(t_vec2 point, t_rect rect);
bool is_rect_in_rect(t_rect this, t_rect that);
bool does_rect_overlap_rect(t_rect this, t_rect that);