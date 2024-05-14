#version 330 core

layout (location = 0) in vec4 vertex;

out vec2 tex_coords;
out vec4 texture_slice;

uniform mat4 u_mat4_projection;

uniform mat4 u_mat4_models[128];
uniform vec4 u_texture_slices[128];

void main() {

    tex_coords = vertex.zw;
    texture_slice = u_texture_slices[gl_InstanceID];

    gl_Position = u_mat4_projection * u_mat4_models[gl_InstanceID] * vec4(vertex.xy, 0.0, 1.0);
}
