#include "t_shapes.h"
#include "t_sprite.h"

#include "../../dep/include/glad/glad.h"

static t_texture s_white_texture;
static t_sprite s_white_rectangle_sprite;

static void create_white_texture() {

    unsigned char white_pixel[] = { 255, 255, 255, 255 };

    unsigned int s_white_texture_id;
    glGenTextures(1, &s_white_texture_id);
    glBindTexture(GL_TEXTURE_2D, s_white_texture_id);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white_pixel);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, 0);

    s_white_texture.channels = 3;
    s_white_texture.id = s_white_texture_id;
    s_white_texture.size.x = 1;
    s_white_texture.size.y = 1;
}

void init_shapes_renderer() {
    create_white_texture();

    create_sprite_t(&s_white_texture, &s_white_rectangle_sprite);
} 

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

void draw_rect(int x, int y, int width, int height, t_color color) {
    draw_sprite(&s_white_rectangle_sprite, x, y, width, height, color);
}