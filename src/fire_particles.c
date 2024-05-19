#include "./engine/tailored.h"
#include "./engine/extern/miniaudio.h"

#include "../dep/include/glad/glad.h"
#include "../dep/include/cglm/cglm.h"
#include "engine/t_core.h"
#include "engine/t_sprite.h"
#include "engine/t_texture.h"

#include <stdlib.h>
#include <math.h>
#include <unistd.h>

/* Function to linearly interpolate between a0 and a1
 * Weight w should be in the range [0.0, 1.0]
 */
float interpolate(float a0, float a1, float w) {
     return (a1 - a0) * ((w * (w * 6.0 - 15.0) + 10.0) * w * w * w) + a0;
}

typedef struct {
    float x, y;
} vector2;

/* Create pseudorandom direction vector
 */
vector2 randomGradient(int ix, int iy) {
    // No precomputed gradients mean this works for any number of grid coordinates
    const unsigned w = 8 * sizeof(unsigned);
    const unsigned s = w / 2; // rotation width
    unsigned a = ix, b = iy;
    a *= 3284157443; b ^= a << s | a >> w-s;
    b *= 1911520717; a ^= b << s | b >> w-s;
    a *= 2048419325;
    float random = a * (3.14159265 / ~(~0u >> 1)); // in [0, 2*Pi]
    vector2 v;
    v.x = cos(random);
    v.y = sin(random);
    return v;
}

// Computes the dot product of the distance and gradient vectors.
float dotGridGradient(int ix, int iy, float x, float y) {
    // Get gradient from integer coordinates
    vector2 gradient = randomGradient(ix, iy);

    // Compute the distance vector
    float dx = x - (float)ix;
    float dy = y - (float)iy;

    // Compute the dot-product
    return (dx*gradient.x + dy*gradient.y);
}

// Compute Perlin noise at coordinates x, y
float perlin(float x, float y) {
    // Determine grid cell coordinates
    int x0 = (int)floor(x);
    int x1 = x0 + 1;
    int y0 = (int)floor(y);
    int y1 = y0 + 1;

    // Determine interpolation weights
    // Could also use higher order polynomial/s-curve here
    float sx = x - (float)x0;
    float sy = y - (float)y0;

    // Interpolate between grid point gradients
    float n0, n1, ix0, ix1, value;

    n0 = dotGridGradient(x0, y0, x, y);
    n1 = dotGridGradient(x1, y0, x, y);
    ix0 = interpolate(n0, n1, sx);

    n0 = dotGridGradient(x0, y1, x, y);
    n1 = dotGridGradient(x1, y1, x, y);
    ix1 = interpolate(n0, n1, sx);

    value = interpolate(ix0, ix1, sy);
    return value * .5f + .5f; // Will return in range -1 to 1. To make it in range 0 to 1, multiply by 0.5 and add 0.5
}

static float random_float(float from, float to) {
    return from + ((float)rand() / RAND_MAX) * (to - from);
}

typedef struct t_particle {

    float life_current;
    float life_initial;

    float size_current;
    float size_initial;

    float timer;

    float color_green_add;

    t_rect rect;

    t_vec2 velocity;

} t_particle;

static t_list* s_list_particles;

static GLuint s_shader_particles;
static GLuint s_quad_vao_particles;
static GLuint s_vertex_buffer_object;

static t_vec2 s_window_size;

static ma_sound s_fire_crackles_sound;
static bool s_should_update;

static mat4 s_mat4_projection;

static void init_quad() {

    float vertices[] = {
        0.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
    };

    glGenVertexArrays(1, &s_quad_vao_particles);
    glGenBuffers(1, &s_vertex_buffer_object);

    glBindBuffer(GL_ARRAY_BUFFER, s_vertex_buffer_object);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(s_quad_vao_particles);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

static void init_particle(t_particle* particle) {

    particle->life_initial = random_float(2, 10);
    particle->life_current = particle->life_initial;
    particle->timer = 0;

    float size = random_float(3, 16);
    particle->size_initial = size;
    particle->size_current = size;

    particle->color_green_add = random_float(0.0f, 2.5f);

    particle->rect = (t_rect) { random_float(-s_window_size.x / 3, s_window_size.x / 2), s_window_size.y, size, size };
}

static t_texture s_load_texture(const t_texture_data* texture_data) {

  GLenum texture_format = 0;
  if (texture_data->channels == 1)
    texture_format = GL_RED;
  else if (texture_data->channels == 3)
    texture_format = GL_RGB;
  else if (texture_data->channels == 4)
    texture_format = GL_RGBA;

  unsigned int texture_id;

  glGenTextures(1, &texture_id);
  glBindTexture(GL_TEXTURE_2D, texture_id);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glTexImage2D(GL_TEXTURE_2D, 0, texture_format, texture_data->width, texture_data->height, 0,
              texture_format, GL_UNSIGNED_BYTE, texture_data->bytes);

  glBindTexture(GL_TEXTURE_2D, 0);

  t_texture texture;
  texture.id = texture_id;

  return texture;
}

static t_sprite s_sprite_perlin;

void init_fire_particles(unsigned int count_particles_max) {

    s_should_update = true;
    ma_result result = t_init_sound("./res/audio/fire_crackles.wav", &s_fire_crackles_sound);

    if (result == MA_SUCCESS) {
        t_start_sound(&s_fire_crackles_sound);
        ma_sound_set_looping(&s_fire_crackles_sound, MA_TRUE);
    }

    s_window_size = t_window_size();

    init_quad();
    glm_ortho(0, s_window_size.x, s_window_size.y, 0, 0.0, 1.0, s_mat4_projection);

    s_list_particles = create_list(sizeof(t_particle));
    s_shader_particles = t_create_shader_program("./res/shaders/fire_particle_shader.vs", "./res/shaders/fire_particle_shader.fs");

    for (unsigned int i = 0; i < count_particles_max; i++) {

        t_particle* particle = (t_particle*)malloc(sizeof(t_particle));
        init_particle(particle);

        particle->rect.x = s_window_size.x;
        particle->life_current /= 2;

        add_to_list(s_list_particles, particle);
    }

    const float perlinx_tex_w = 256;
    const float perlinx_tex_h = 256;

    unsigned char pixels[256][256][4];

    float x_org = 0;
    float y_org = 0;
    float scale = 5;

    for (int y = 0; y < perlinx_tex_h; y++) {
        for (int x = 0; x < perlinx_tex_w; x++) {
            float x_coord = x_org + (float)x / perlinx_tex_w * scale;
            float y_coord = y_org + (float)y / perlinx_tex_h * scale;

            float sample = perlin(x_coord, y_coord);
            pixels[x][y][0] = 255 * sample;
            pixels[x][y][1] = 255 * sample;
            pixels[x][y][2] = 255 * sample;
            pixels[x][y][3] = 255;
        }
    }

    t_texture_data texture_data = { .bytes = (unsigned char*) pixels, .width = 256, .height = 256, .channels = 4 };
    t_texture texture = s_load_texture(&texture_data);

    s_sprite_perlin.texture_data = texture_data;
    s_sprite_perlin.texture = texture;
    s_sprite_perlin.scale = VEC2_ONE;
    s_sprite_perlin.texture_slice = (t_vec4) {0, 0, 256, 256};
    s_sprite_perlin.slice_borders = VEC4_ZERO;
}

void uninit_fire_particles() {

    t_uninit_sound(&s_fire_crackles_sound);
    destroy_list(s_list_particles);

    t_destroy_shader_program(s_shader_particles);
    glDeleteBuffers(1, &s_vertex_buffer_object);
    glDeleteVertexArrays(1, &s_quad_vao_particles);
}
float x_org_u = 0;

void draw_fire_particles() {

    glUseProgram(s_shader_particles);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_COLOR);

    for (unsigned int i = 0; i < s_list_particles->size; i ++) {

        t_particle* particle = (t_particle*)element_at_list(s_list_particles, i);

        if (s_should_update && particle->life_current <= 0) {
           init_particle(particle);
        }

        mat4 mat4_model;
        glm_mat4_identity(mat4_model);
        glm_translate(mat4_model,
                    (vec3){particle->rect.x, particle->rect.y, -0.1f});
        glm_scale(mat4_model, (vec3){particle->rect.width, particle->rect.height, 1.0f});

        glUniformMatrix4fv(glGetUniformLocation(s_shader_particles, "u_mat4_projection"),
                            1, GL_FALSE, (float *)s_mat4_projection);

        char mat4_model_key[20];
        sprintf(mat4_model_key, "u_mat4_models[%d]", i);

        glUniformMatrix4fv(glGetUniformLocation(s_shader_particles, mat4_model_key), 1,
                            GL_FALSE, (float *)mat4_model);

        char green_add_key[20];
        sprintf(green_add_key, "u_green_add[%d]", i);
        glUniform1f(glGetUniformLocation(s_shader_particles, green_add_key), particle->color_green_add);

        if (particle->life_current > 0) {
            particle->life_current -= t_delta_time();

            // float x_coord = (float)particle->rect.x / s_window_size.x * 10;
            // float y_coord = t_get_time() + (float)particle->rect.y / s_window_size.y * 10;

            // float sample = perlin(x_coord, y_coord);

            // particle->velocity.x = 900 * sample;
            // particle->velocity.y = 350;

            // particle->rect.x += particle->velocity.x * t_delta_time();
            // particle->rect.y -= particle->velocity.y * t_delta_time();

            particle->rect.x += random_float(-10, 500) * t_delta_time() * (particle->rect.width * 0.2f);
            particle->rect.y -= random_float(-10, 450) * t_delta_time() * (particle->rect.height * 0.2f);

            float size = 0;
            t_ease_out_quint(&particle->timer, &size, particle->size_initial, 0, particle->life_initial);

            particle->rect.width = size;
            particle->rect.height = size;
        }
    }

    glBindVertexArray(s_quad_vao_particles);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, s_list_particles->size);
    glBindVertexArray(0);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void set_updating(bool value) {
    s_should_update = value;

    if (!value) {
        t_fade_out_sound(&s_fire_crackles_sound, 1);
    } else {
        t_fade_in_sound(&s_fire_crackles_sound, 1);
    }
}
