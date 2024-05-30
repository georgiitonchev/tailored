#version 330 core

layout (location = 0) in vec4 vertex;

out vec2 tex_coords;

uniform mat4 u_mat4_model;
uniform mat4 u_mat4_projection;

void main() {
    tex_coords = vertex.zw;
    vec4 position = u_mat4_projection * u_mat4_model * vec4(vertex.xy, 0.0, 1.0);
    
    gl_Position = position;
}
