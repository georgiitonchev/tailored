#version 330 core

in vec2 tex_coords;
out vec4 color;

uniform sampler2D u_texture;
uniform vec3 u_color;

void main() {
    color = vec4(u_color, 1.0) * texture(u_texture, tex_coords);
}
