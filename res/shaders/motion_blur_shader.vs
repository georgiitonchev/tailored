#version 330 core

layout (location = 0) in vec4 vertex;

out vec2 tex_coords;

void main() {

    tex_coords = vertex.zw;

    gl_Position = vec4(vertex.x, vertex.y, 0.0, 1.0);
}