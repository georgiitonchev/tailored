#version 330 core

in vec3 normal;
in vec3 frag_pos;
in vec2 tex_coord;

uniform sampler2D u_texture_base;
uniform vec3 u_color_base;
uniform bool u_use_base_color;

out vec4 fragment;

void main() {
    if (u_use_base_color) {
        fragment = vec4(u_color_base, 1.0);
    }
    else {
        fragment = texture(u_texture_base, tex_coord);
    }
}
