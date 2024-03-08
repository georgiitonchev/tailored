#version 330

in vec3 vCol;
in vec2 vPos;

out vec3 color;

uniform mat4 MVP;

void main(){
    gl_Position = MVP * vec4(vPos, 0.0, 1.0);
    color = vCol;
}
