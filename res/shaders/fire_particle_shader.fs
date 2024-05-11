#version 330 core

in vec2 tex_coords;

out vec4 color;

uniform float u_green_add;

void main() {

    color = vec4(.66f, .11f * (1 + u_green_add), .15f, .8f);
}
