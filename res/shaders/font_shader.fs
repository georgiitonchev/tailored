#version 330 core

in vec2 tex_coords;
in vec4 texture_slice;

out vec4 color;

uniform sampler2D u_texture;
uniform vec4 u_color;

float map(float value, float from_min, float from_max, float to_min, float to_max) {
    return (value - from_min) / (from_max - from_min) * (to_max - to_min) + to_min;
} 

void main() {
    vec2 texture_size = textureSize(u_texture, 0);

    vec2 tex_coord = tex_coords;

    if (texture_slice.z != 0 && texture_slice.w != 0) {
        vec2 s_slice = texture_size / texture_slice.zw;

        float s_offset_x = 0;
        float s_offset_y = 0;

        if (texture_slice.x > 0) {
            s_offset_x = 1 / (texture_size.x / texture_slice.x);
        } 

        if (texture_slice.y > 0) {
            s_offset_y = 1 / (texture_size.y / texture_slice.y);
        }

        tex_coord.x = map(tex_coord.x, 
                        0, 1,
                        s_offset_x, s_offset_x + (1 / s_slice.x));

       tex_coord.y = map(tex_coord.y, 
                        0, 1,
                        s_offset_y, s_offset_y + (1 / s_slice.y));
    }


    color = u_color * vec4(1.0, 1.0, 1.0, texture(u_texture, tex_coord).r);
}
