#version 330 core

in vec3 color;
in vec2 tex_coord;

out vec4 fragment;

uniform sampler2D u_texture_a;
uniform sampler2D u_texture_b;

void main(){
    fragment = mix(texture(u_texture_a, tex_coord), texture(u_texture_b, tex_coord), 0.2);
}
