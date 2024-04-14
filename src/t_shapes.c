#include "t_shapes.h"

bool is_point_in_rect(t_vec2 point, t_rect rect) {
    return point.x >= rect.x && point.x <= rect.x + rect.width && point.y >= rect.y && point.y <= rect.y + rect.height;
}

