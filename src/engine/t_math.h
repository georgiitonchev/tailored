float perlin(float x, float y);

t_vec2 t_vec2_sub(t_vec2 a, t_vec2 b);
float t_vec2_distance(t_vec2 a, t_vec2 b);
float t_vec2_angle(t_vec2 a, t_vec2 b);
t_vec2 t_vec2_dir(t_vec2 a, t_vec2 b);
float t_vec2_dot(t_vec2 a, t_vec2 b);
t_vec2 t_vec2_normalize(t_vec2 v);
t_vec2 t_vec2_lerp(t_vec2 a, t_vec2 b, float t);

float t_map(float value, float from_min, float from_max, float to_min, float to_max);
float t_random_float(float from, float to);
int t_random_int(int from, int to);