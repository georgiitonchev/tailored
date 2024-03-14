#version 330 core

struct DirectionalLight {
    vec3 color;
    vec3 position;
    vec3 direction;
};

struct PointLight {
    vec3 color;
    vec3 position;

    float constant;
    float linear;
    float quadratic;
};

struct SpotLight {
    vec3 color;
    vec3 position;
    vec3 direction;

    float constant;
    float linear;
    float quadratic;

    float outer_angle;
    float inner_angle;
};

in vec3 normal;
in vec3 frag_pos;
in vec2 tex_coord;

out vec4 fragment;

uniform vec3 u_object_color;
uniform vec3 u_cam_pos;

uniform sampler2D u_texture;
uniform sampler2D u_texture_specular;
uniform sampler2D u_texture_emission;

uniform DirectionalLight u_dir_light;
uniform PointLight u_point_light;
uniform SpotLight u_spot_light;

vec3 calc_dir_light();
vec3 calc_point_light(PointLight);
vec3 calc_spot_light(SpotLight);

void main(){
    vec3 dir_light_result = calc_dir_light();
    vec3 point_light_result = calc_point_light(u_point_light);
    vec3 spot_light_result = calc_spot_light(u_spot_light);

    vec3 result = dir_light_result;
    result += point_light_result;
    result += spot_light_result;

    fragment = vec4(result, 1);
}

vec3 calc_dir_light()
{
    float ambient_strength = 0.1;
    vec3 ambient = ambient_strength * u_dir_light.color;

    vec3 norm = normalize(normal);
    vec3 light_dir = normalize(u_dir_light.direction);

    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = diff * u_dir_light.color;

    float specular_strength = 0.5;
    vec3 view_dir = normalize(u_cam_pos - frag_pos);
    vec3 reflect_dir = reflect(-light_dir, norm);

    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 32);
    vec3 specular = specular_strength * spec * u_dir_light.color;

    vec3 result = (ambient + diffuse) * vec3(texture(u_texture, tex_coord));
    result += specular * vec3(texture(u_texture_specular, tex_coord));
    return result;
}

vec3 calc_point_light(PointLight point_light)
{
    vec3 light_dir = normalize(u_spot_light.position - frag_pos);

    // diffuse shading
    float diff = max(dot(normal, light_dir), 0.0);
    // specular shading

    vec3 view_dir = normalize(u_cam_pos - frag_pos);
    vec3 reflect_dir = reflect(-light_dir, normal);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 32);
    // attenuation
    float distance = length(point_light.position - frag_pos);
    float attenuation = 1.0 / (point_light.constant + point_light.linear * distance +
      			     point_light.quadratic * (distance * distance));

    vec3 ambient  = point_light.color * vec3(texture(u_texture, tex_coord));
    vec3 diffuse  = point_light.color * diff * vec3(texture(u_texture, tex_coord));
    vec3 specular = point_light.color * spec * vec3(texture(u_texture_specular, tex_coord));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

vec3 calc_spot_light(SpotLight spot_light){

    vec3 light_dir = normalize(spot_light.position - frag_pos);

    float diff = max(dot(normal, light_dir), 0.0);
    vec3 view_dir = normalize(u_cam_pos - frag_pos);
    vec3 reflect_dir = reflect(-light_dir, normal);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 32);
    // attenuation
    float distance = length(spot_light.position - frag_pos);
    float attenuation = 1.0 / (spot_light.constant + spot_light.linear * distance +
      			     spot_light.quadratic * (distance * distance));

    vec3 ambient = spot_light.color * vec3(texture(u_texture, tex_coord));
    vec3 diffuse  = spot_light.color * diff * vec3(texture(u_texture, tex_coord));
    vec3 specular = spot_light.color * spec * vec3(texture(u_texture_specular, tex_coord));

    ambient *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

    float theta = dot(light_dir, normalize(-spot_light.direction));
    float epsilon  = spot_light.inner_angle - spot_light.outer_angle;
    float intensity = clamp((theta - spot_light.outer_angle) / epsilon, 0.0, 1.0);

    ambient *= intensity;
    diffuse *= intensity;
    specular *= intensity;

    return (ambient + diffuse + specular);
}
