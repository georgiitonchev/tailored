#version 330 core

layout (location = 0) in vec3 in_pos;

out vec4 vertex_color;

void main(){

    gl_Position = vec4(in_pos, 1.0);
    vertex_color = vec4(0.5, 0.0, 0.0, 1.0);
}
