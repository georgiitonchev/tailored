#version 330 core

in vec2 tex_coords;

out vec4 color;

uniform sampler2D u_texture;
uniform mat4 u_mat4_model;
uniform vec4 u_color;
uniform vec4 u_slice_borders;
uniform vec4 u_texture_slice;

float map(float value, float from_min, float from_max, float to_min, float to_max) {
    return (value - from_min) / (from_max - from_min) * (to_max - to_min) + to_min;
} 

void main() {

    vec2 texture_size = textureSize(u_texture, 0);

    vec2 size;
    size.x = length(u_mat4_model[0].xyz);
    size.y = length(u_mat4_model[1].xyz);

    vec2 tex_coord = tex_coords;

    //left
    if (tex_coord.x < u_slice_borders.x / size.x) {
        tex_coord.x = map(tex_coord.x, 
            0, u_slice_borders.x / size.x, //from
            0, u_slice_borders.x / texture_size.x); //to
    }
    //right
    else if (tex_coord.x > (size.x - u_slice_borders.y) / size.x) {
        tex_coord.x = map(tex_coord.x,
            (size.x - u_slice_borders.y) / size.x, 1, //from
            (texture_size.x - u_slice_borders.y) /texture_size.x, 1);
    }
    else {
        tex_coord.x = map(tex_coord.x, 
            0, 1,
            u_slice_borders.x / texture_size.x, (texture_size.x - u_slice_borders.y) /texture_size.x);
    }

    //top
    if (tex_coord.y < u_slice_borders.z / size.y) {
        tex_coord.y = map(tex_coord.y, 
            0, u_slice_borders.z / size.y, //from
            0, u_slice_borders.z / texture_size.y); //to
    }

    //bot
    else if (tex_coord.y > (size.y - u_slice_borders.w) / size.y) {
         tex_coord.y = map(tex_coord.y,
            (size.y - u_slice_borders.w) / size.y, 1, //from
            (texture_size.y - u_slice_borders.w) / texture_size.y, 1);
    }
    else {
         tex_coord.y = map(tex_coord.y,
            0, 1,
            u_slice_borders.z / texture_size.y, (texture_size.y - u_slice_borders.w) / texture_size.y);
    }

    if (u_texture_slice.z != 0 && u_texture_slice.w != 0) {
        vec2 s_slice = texture_size / u_texture_slice.zw;

        float s_offset_x = 0;
        float s_offset_y = 0;

        if (u_texture_slice.x > 0) {
            s_offset_x = 1 / (texture_size.x / u_texture_slice.x);
        } 

        if (u_texture_slice.y > 0) {
            s_offset_y = 1 / (texture_size.y / u_texture_slice.y);
        }

        tex_coord.x = map(tex_coord.x, 
                        0, 1,
                        s_offset_x, s_offset_x + (1 / s_slice.x));

       tex_coord.y = map(tex_coord.y, 
                        0, 1,
                        s_offset_y, s_offset_y + (1 / s_slice.y));
    }

    color = u_color * texture(u_texture, tex_coord);

}
