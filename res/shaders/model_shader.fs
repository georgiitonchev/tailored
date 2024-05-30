#version 330 core

in vec3 normal;
in vec3 frag_pos;
in vec2 tex_coord;
in vec4 frag_pos_light_space;

uniform sampler2D u_shadow_map;

uniform sampler2D u_texture_base;
uniform vec3 u_color_base;
uniform bool u_use_base_color;

uniform vec3 light_pos;
uniform vec3 view_pos;
uniform vec3 light_color;

uniform bool use_blinn;

out vec4 fragment;

float calculate_shadow() {

    // perform perspective divide
    vec3 projCoords = frag_pos_light_space.xyz / frag_pos_light_space.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(u_shadow_map, projCoords.xy).r;
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    float bias = 0.001;
    float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;

    return shadow;
}

vec4 calculate_phong_lighting() {

    // ambient
    float ambient_strength = 0.1;
    vec3 ambient = ambient_strength * light_color;

    // diffuse
    vec3 norm = normalize(normal);
    vec3 light_dir = normalize(light_pos - frag_pos);
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = diff * light_color;

    // specular
    float specular_strength = 0.5;
    vec3 view_dir = normalize(view_pos - frag_pos);

    float spec = 0.0;
    if (use_blinn)
    {
        vec3 halfway_dir = normalize(light_dir + view_dir);
        spec = pow(max(dot(normal, halfway_dir), 0.0), 32.0);
    }
    else
    {
        vec3 reflect_dir = reflect(-light_dir, normal);
        spec = pow(max(dot(view_dir, reflect_dir), 0.0), 8.0);
    }
    vec3 specular = specular_strength * spec * light_color;

    return vec4(ambient + (1.0 - calculate_shadow()) * (diffuse + specular), 1.0);
}

void main() {

    vec4 fragment_color;

    if (u_use_base_color)
        fragment_color = vec4(u_color_base, 1.0);
    else
        fragment_color = texture(u_texture_base, tex_coord);

    vec4 result = calculate_phong_lighting() * fragment_color;

    fragment = result;
}
