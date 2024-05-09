#include "t_shapes.h"
#include "t_sprite.h"

#include "../../dep/include/glad/glad.h"

static t_sprite s_white_rectangle_sprite;
static t_sprite s_white_border_rectangle_sprite;

static t_texture create_white_texture() {

    unsigned char white_pixel[] = { 255, 255, 255, 255 };

    unsigned int s_white_texture_id;
    glGenTextures(1, &s_white_texture_id);
    glBindTexture(GL_TEXTURE_2D, s_white_texture_id);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white_pixel);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, 0);

    t_texture texture;
    texture.channels = 3;
    texture.id = s_white_texture_id;
    texture.size.x = 1;
    texture.size.y = 1;

    return texture;
}

static t_texture create_white_border_texture() {

    unsigned char pixels[3][3][4]; // RGBA format
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            if (i == 1 && j == 1) {
                pixels[i][j][0] = 0;   // R
                pixels[i][j][1] = 0;   // G
                pixels[i][j][2] = 0;   // B
                pixels[i][j][3] = 0;   // A (Transparent)
            } else {
                pixels[i][j][0] = 255; // R
                pixels[i][j][1] = 255; // G
                pixels[i][j][2] = 255; // B
                pixels[i][j][3] = 255; // A (Opaque)
            }
        }
    }

    unsigned int s_white_border_texture_id;
    glGenTextures(1, &s_white_border_texture_id);
    glBindTexture(GL_TEXTURE_2D, s_white_border_texture_id);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 3, 3, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, 0);

    t_texture texture;
    texture.channels = 3;
    texture.id = s_white_border_texture_id;
    texture.size.x = 3;
    texture.size.y = 3;

    return texture;
}

void init_shapes_renderer() {
    t_texture white_texture = create_white_texture();
    create_sprite_t(&white_texture, &s_white_rectangle_sprite);

    t_texture white_border_texture = create_white_border_texture();
    create_sprite_t(&white_border_texture, &s_white_border_rectangle_sprite);
    s_white_border_rectangle_sprite.slice_borders = VEC4_ONE;
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

void draw_rect_lines(int x, int y, int width, int height, t_color color) {
    draw_sprite(&s_white_border_rectangle_sprite, x, y, width, height, color);
}
