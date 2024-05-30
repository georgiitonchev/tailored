#version 330 core

layout (location = 0) in vec2 vertex_pos;

uniform float u_green_add[100];
uniform mat4 u_mat4_models[100];
uniform mat4 u_mat4_projection;

out float green_add;

void main() {

    green_add = u_green_add[gl_InstanceID];
    gl_Position = u_mat4_projection * u_mat4_models[gl_InstanceID] * vec4(vertex_pos, 0.0, 1.0);
}
