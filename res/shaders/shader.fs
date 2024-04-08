#version 330 core

in vec2 tex_coords;

out vec4 color;

uniform sampler2D u_texture;
uniform mat4 u_mat4_model;
uniform vec4 u_color;
uniform vec4 u_slice_borders;

float map(float value, float from_min, float from_max, float to_min, float to_max) {
    return (value - from_min) / (from_max - from_min) * (to_max - to_min) + to_min;
} 

void main() {

    vec2 texture_size = textureSize(u_texture, 0);

    vec2 size;
    size.x = length(u_mat4_model[0].xyz);
    size.y = length(u_mat4_model[1].xyz);

    vec4 debug_color = vec4(1.0, 1.0, 1.0, 1.0);
    vec2 tex_coord = tex_coords;

    //left
    if (tex_coord.x < u_slice_borders.x / size.x) {
        tex_coord.x = map(tex_coord.x, 
            0, u_slice_borders.x / size.x, //from
            0, u_slice_borders.x / texture_size.x); //to
    }
    //right
    else if (tex_coord.x > (size.x - u_slice_borders.y) / size.x) {
        tex_coord.x = map(tex_coord.x,
            (size.x - u_slice_borders.y) / size.x, 1, //from
            (texture_size.x - u_slice_borders.y) /texture_size.x, 1);
    }
    else {
        tex_coord.x = map(tex_coord.x, 
            0, 1,
            u_slice_borders.x / texture_size.x, (texture_size.x - u_slice_borders.y) /texture_size.x);
    }

    //top
    if (tex_coord.y < u_slice_borders.z / size.y) {
        tex_coord.y = map(tex_coord.y, 
            0, u_slice_borders.z / size.y, //from
            0, u_slice_borders.z / texture_size.y); //to
    }

    //bot
    else if (tex_coord.y > (size.y - u_slice_borders.w) / size.y) {
         tex_coord.y = map(tex_coord.y,
            (size.y - u_slice_borders.w) / size.y, 1, //from
            (texture_size.y - u_slice_borders.w) / texture_size.y, 1);
    }
    else {
         tex_coord.y = map(tex_coord.y,
            0, 1,
            u_slice_borders.z / texture_size.y, (texture_size.y - u_slice_borders.w) / texture_size.y);
    }

    color = debug_color* u_color * texture(u_texture, tex_coord);

}
