#version 330 core

in float green_add;

out vec4 color;

void main() {

    color = vec4(.66f, .11f * (1 + green_add), .15f, .8f);
}
