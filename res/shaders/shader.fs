#version 330 core

in vec3 color;
in vec2 tex_coord;

out vec4 fragment;

uniform sampler2D u_texture;

void main(){
    fragment = texture(u_texture, tex_coord);
}
