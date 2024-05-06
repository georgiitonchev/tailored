#version 330

in vec2 tex_coords;
in vec4 color;

uniform sampler2D u_texture;

out vec4 out_color;

void main()
{
    float distance_from_outline = texture(u_texture, tex_coords).a - 0.5;
    float distance_change_per_fragment = length(vec2(dFdx(distance_from_outline), dFdy(distance_from_outline)));
    float alpha = smoothstep(-distance_change_per_fragment, distance_change_per_fragment, distance_from_outline);

    out_color = vec4(color.rgb, color.a * alpha);
}