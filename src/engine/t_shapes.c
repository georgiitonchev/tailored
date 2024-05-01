#include "t_shapes.h"

bool is_point_in_rect(t_vec2 point, t_rect rect) {
    return point.x >= rect.x && point.x <= rect.x + rect.width && point.y >= rect.y && point.y <= rect.y + rect.height;
}

bool is_rect_in_rect(t_rect this, t_rect that) {
    return this.x >= that.x && this.x + this.width <= that.x + that.width &&
           this.y >= that.y && this.y + this.height <= that.y + that.height;
}

bool does_rect_overlap_rect(t_rect this, t_rect that) {

    if (this.x + this.width <= that.x || that.x + that.width <= this.x)
        return false;

    if (this.y + this.height <= that.y || that.y + that.height <= this.y)
        return false;

    return true;
}