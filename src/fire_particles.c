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

static t_list* particles_list;

static unsigned int particles_shader;
static unsigned int particle_quad_vao;

static t_vec2 window_size; 

static ma_sound s_fire_crackles_sound;
static bool s_should_update;

static void init_quad() {
    unsigned int vertex_buffer_object;

    float vertices[] = {
        0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,

        0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f};

    glGenVertexArrays(1, &particle_quad_vao);
    glGenBuffers(1, &vertex_buffer_object);

    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(particle_quad_vao);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
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

    particle->rect = (t_rect) { random_float(-window_size.x / 3, window_size.x / 2), window_size.y, size, size };
}

void draw_particle(t_particle* particle) { 

    mat4 mat4_projection;
    glm_ortho(0, window_size.x, window_size.y, 0, 0.0, 1.0, mat4_projection);

    mat4 mat4_model;
    glm_mat4_identity(mat4_model);
    glm_translate(mat4_model,
                (vec3){particle->rect.x, particle->rect.y, -0.1f});
    glm_scale(mat4_model, (vec3){particle->rect.width, particle->rect.height, 1.0f});

    glUniformMatrix4fv(glGetUniformLocation(particles_shader, "u_mat4_projection"),
                        1, GL_FALSE, (float *)mat4_projection);

    glUniformMatrix4fv(glGetUniformLocation(particles_shader, "u_mat4_model"), 1,
                        GL_FALSE, (float *)mat4_model);

    glUniform1f(glGetUniformLocation(particles_shader, "u_green_add"), particle->color_green_add);

    glBindVertexArray(particle_quad_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void init_fire_particles(unsigned int count_particles_max) {

    s_should_update = true;
    ma_result result = t_init_sound("./res/audio/fire_crackles.wav", &s_fire_crackles_sound);

    if (result == MA_SUCCESS) {
        t_start_sound(&s_fire_crackles_sound);
        ma_sound_set_looping(&s_fire_crackles_sound, MA_TRUE);
    }
    
    window_size = t_window_size();

    init_quad();

    particles_list = create_list(sizeof(t_particle));
    particles_shader = t_create_shader_program("./res/shaders/fire_particle_shader.vs", "./res/shaders/fire_particle_shader.fs");

    for (unsigned int i = 0; i < count_particles_max; i++) {

        t_particle* particle = (t_particle*)malloc(sizeof(t_particle));
        init_particle(particle);
        add_to_list(particles_list, particle);
    }
}

void draw_fire_particles() { 
    
    glUseProgram(particles_shader);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_COLOR);  

    for (int i = 0; i < particles_list->size; i ++) { 

        t_particle* particle = (t_particle*)element_at_list(particles_list, i);
        
        if (s_should_update && particle->life_current <= 0) { 
           init_particle(particle);
        }

        if (particle->life_current > 0) {
            draw_particle(particle);

            particle->life_current -= t_delta_time();
            particle->rect.x += random_float(-10, 500) * t_delta_time() * (particle->rect.width * 0.2f);
            particle->rect.y -= random_float(-10, 450) * t_delta_time() * (particle->rect.height * 0.2f); 

            float size = 0;
            t_ease_out_quint(&particle->timer, &size, particle->size_initial, 0, particle->life_initial);

            particle->rect.width = size;
            particle->rect.height = size;
        }
    }

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
}

void set_updating(bool value) {
    printf("set updateing");
    s_should_update = value;

    if(!value) { 
        t_fade_out_sound(&s_fire_crackles_sound, 1);
    }
}