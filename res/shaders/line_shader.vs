#version 330 core
layout (location = 0) in vec2 vertex;

void main() {

    vec4 position = vec4(vertex, 0.0, 1.0);
    gl_Position = position;
}
