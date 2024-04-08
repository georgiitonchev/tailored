#version 330 core

in vec2 tex_coords;

out vec4 color;

uniform sampler2D u_texture;
uniform mat4 u_mat4_model;
uniform vec4 u_color;
uniform vec4 u_slice_borders;

void main() {

    vec2 texture_size = textureSize(u_texture, 0);

    vec2 scale;
    scale.x = length(u_mat4_model[0].xyz) / texture_size.x;
    scale.y = length(u_mat4_model[1].xyz) / texture_size.y;

    vec4 debug_color = vec4(1.0, 1.0, 1.0, 1.0);
    vec2 tex_coord = tex_coords;
    bool within_borders = false;
    bool border_horizontal = false;
    bool border_vertical = false;

    //left border
    float border_left = (u_slice_borders.x / texture_size.x) / scale.x;
    if (tex_coords.x < border_left) {
        tex_coord.x = tex_coords.x * scale.x;
        within_borders = true;
        border_horizontal = true;
    }

    //right border
    float border_right = (texture_size.x - u_slice_borders.y) / texture_size.x;
    border_right = border_right + (1 - border_right) / scale.x;
    if (tex_coords.x > border_right) {
        //tex_coord.x = tex_coords.x * scale.x;
        within_borders = true;
        border_horizontal = true;
        debug_color = vec4(0,1,0,1);
    }

    //top border
    float border_top = (u_slice_borders.z / texture_size.y) / scale.y;
    if (tex_coords.y < border_top) {
        tex_coord.y = tex_coords.y * scale.y;
        within_borders = true;
        border_vertical = true;
    }

    //bottom border
    float border_bottom = (texture_size.y - u_slice_borders.w) / texture_size.y;
    border_bottom = border_bottom + (1 - border_bottom) / scale.y;
    if (tex_coords.y > border_bottom) {
        //tex_coord.y = tex_coords.y * scale.y;
        within_borders = true;
        border_vertical = true;
        debug_color = vec4(0,1,0,1);
    }

    if (!border_horizontal)
        tex_coord.x = clamp(tex_coord.x, (border_left + 0.01) * scale.x, border_right / scale.x);

    if (!border_vertical)
        tex_coord.y = clamp(tex_coord.y, (border_top + 0.01) * scale.y, border_bottom / scale.y);

    color = debug_color* u_color * texture(u_texture, tex_coord);

}
