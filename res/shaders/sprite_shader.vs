#version 330 core

layout (location = 0) in vec4 vertex;

out vec2 tex_coords;

out vec4 clip_space_pos;
out vec4 clip_space_pos_prev;

uniform mat4 u_mat4_model;
uniform mat4 u_mat4_projection;

uniform mat4 u_mat4_model_previous;

void main() {

    tex_coords = vertex.zw;
    vec4 position = u_mat4_projection * u_mat4_model * vec4(vertex.xy, 0.0, 1.0);

    clip_space_pos = position;
    clip_space_pos_prev = u_mat4_projection * u_mat4_model_previous * vec4(vertex.xy, 0.0, 1.0);

    gl_Position = position;
}
