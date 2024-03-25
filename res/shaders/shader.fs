#version 330 core

in vec3 normal;
in vec3 frag_pos;
in vec2 tex_coord;

out vec4 fragment;

void main(){

    fragment = vec4(normal, 1.0);
}
