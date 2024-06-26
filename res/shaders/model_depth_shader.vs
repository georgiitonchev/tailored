#version 330 core
layout (location = 0) in vec3 pos;

uniform mat4 light_space_matrix;
uniform mat4 u_model;

void main()
{
    gl_Position = light_space_matrix * u_model * vec4(pos, 1.0);
}
