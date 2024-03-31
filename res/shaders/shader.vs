#version 330 core

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_tex_coord;

out vec2 tex_coord;
out vec3 normal;
out vec3 frag_pos;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

void main() {

    gl_Position = u_projection * u_view * u_model * vec4(in_pos, 1.0);

    frag_pos = vec3(u_model * vec4(in_pos, 1));
    //normal = mat3(transpose(inverse(u_model))) * in_normal;
    normal = in_normal;
    tex_coord = in_tex_coord;
}
