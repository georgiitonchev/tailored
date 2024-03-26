#version 330 core

in vec3 normal;
in vec3 frag_pos;
in vec2 tex_coord;

uniform sampler2D u_texture_base;

out vec4 fragment;

void main() {
    fragment = texture(u_texture_base, tex_coord);
}
