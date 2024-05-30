#version 330 core

in vec2 tex_coords;

out vec4 color;

uniform sampler2D u_texture;
uniform sampler2D u_motion_texture;

void main() {

    // vec2 motion_vector = texture(u_motion_texture, tex_coords).xy / 10;
    // vec4 out_color = vec4(0.0);

    // vec2 tex_coord = tex_coords;

    // out_color += texture(u_texture, tex_coord) * 0.4;
    // tex_coord -= motion_vector;
    // out_color += texture(u_texture, tex_coord) * 0.3;
    // tex_coord -= motion_vector;
    // out_color += texture(u_texture, tex_coord) * 0.2;
    // tex_coord -= motion_vector;
    // out_color += texture(u_texture, tex_coord) * 0.1;

    //color = out_color;
    color = texture(u_texture, tex_coords);
}
