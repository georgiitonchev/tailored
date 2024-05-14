#include "./engine/t_list.h"
#include "./engine/tailored.h"
#include "./engine/t_easings.h"
#include "./engine/extern/miniaudio.h"


#include "../dep/include/glad/glad.h"
#include "../dep/include/cglm/cglm.h"

#include <stdlib.h>

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
}

void uninit_fire_particles() {

    t_uninit_sound(&s_fire_crackles_sound);
    destroy_list(s_list_particles);

    t_destroy_shader_program(s_shader_particles);
    glDeleteBuffers(1, &s_vertex_buffer_object);
    glDeleteVertexArrays(1, &s_quad_vao_particles);
}

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
        sprintf_s(mat4_model_key, 20, "u_mat4_models[%d]", i);

        glUniformMatrix4fv(glGetUniformLocation(s_shader_particles, mat4_model_key), 1,
                            GL_FALSE, (float *)mat4_model);

        char green_add_key[20];
        sprintf_s(green_add_key, 20, "u_green_add[%d]", i);
        glUniform1f(glGetUniformLocation(s_shader_particles, green_add_key), particle->color_green_add);

        if (particle->life_current > 0) {
            particle->life_current -= t_delta_time();
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