#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#endif

#include "tailored.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Audio
#define MINIAUDIO_IMPLEMENTATION
#include "./extern/miniaudio.h"

// GL loader
#include "../../dep/include/glad/glad.h"
// Windowing
#include "../../dep/include/GLFW/glfw3.h"
// MATH
#include "../../dep/include/cglm/cglm.h"

#include "t_sprite.h"
#include "t_font.h"
#include "t_shapes.h"

static GLFWwindow* s_window;
static ma_engine s_audio_engine;

static float s_delta_time;
static float s_last_frame_time;

t_input_state input_state;
static t_vec2 window_size;
static t_vec2 framebuffer_size;
// unsigned int mat4_models_prev_index;

static unsigned int frame_buffer_id;
static unsigned int motion_blur_shader_id;

static unsigned int first_pass_texture_id;
static unsigned int motion_vector_texture_id;

t_vec2 t_window_size() { 
  return window_size;
}

t_vec2 t_framebuffer_size() { 
  return framebuffer_size;
}

unsigned int create_framebuffer() {

  // generate framebuffer
  glGenFramebuffers(1, &frame_buffer_id);
  glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_id);

  // generate target texture
  glGenTextures(1, &first_pass_texture_id);
  glBindTexture(GL_TEXTURE_2D, first_pass_texture_id);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 640, 360, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL); // maybe framebuffer size instead?

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);

  // attach to framebuffer
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, first_pass_texture_id, 0);


  // generate motion vector texture
  glGenTextures(1, &motion_vector_texture_id);
  glBindTexture(GL_TEXTURE_2D, motion_vector_texture_id);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 640, 360, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL); // maybe framebuffer size instead?

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);

  // attach to framebuffer
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, motion_vector_texture_id, 0);


  // create a render buffer for depth & stencil
  unsigned int render_buffer_id;
  glGenRenderbuffers(1, &render_buffer_id);
  glBindRenderbuffer(GL_RENDERBUFFER, render_buffer_id); 
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 640, 360);  
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, render_buffer_id);

  GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
  glDrawBuffers(2, drawBuffers);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    printf("ERROR: framebuffer is incomplete.\n");
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);  
  return frame_buffer_id;
}

void bind_framebuffer(unsigned int frame_buffer_id) {
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frame_buffer_id);
}
static unsigned int motion_blur_quad_vao;

static void init_quad() {
  // configure VAO/VBO
  unsigned int vertex_buffer_object;

  float vertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
  };

  glGenVertexArrays(1, &motion_blur_quad_vao);
  glGenBuffers(1, &vertex_buffer_object);

  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glBindVertexArray(motion_blur_quad_vao);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

float t_delta_time() { 
  return s_delta_time;
}

void t_play_audio(const char* path) {
  ma_engine_play_sound(&s_audio_engine, path, NULL);
}

void t_set_cursor(const char* path, int xhot, int yhot) {

  t_texture_data texture_data = t_load_texture_data(path);

  GLFWimage image;
  image.width = texture_data.width;
  image.height = texture_data.height;
  image.pixels = texture_data.data;

  GLFWcursor* cursor = glfwCreateCursor(&image, xhot, yhot);
  glfwSetCursor(s_window, cursor);
 }

static void calculate_delta_time() {
  float current_frame_time = glfwGetTime();
  s_delta_time = current_frame_time - s_last_frame_time;
  s_last_frame_time = current_frame_time;
}

#ifndef __APPLE__
void APIENTRY debug_callback(GLenum source, GLenum type, GLuint id,
  GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
  UNUSED(source);
  UNUSED(type);
  UNUSED(id);
  UNUSED(length);
  UNUSED(userParam);

  const char* severityStr =
    severity == GL_DEBUG_SEVERITY_HIGH ? "High" :
    severity == GL_DEBUG_SEVERITY_MEDIUM ? "Medium" :
    severity == GL_DEBUG_SEVERITY_LOW ? "Low" :
    severity == GL_DEBUG_SEVERITY_NOTIFICATION ? "Notification" : "";

  printf("OpenGL %s : %s\n", severityStr, message);
}
#endif

static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
  UNUSED(window);
  UNUSED(mods);

  // update state
  if (action == GLFW_RELEASE) {
    input_state.mouse_state.buttons[button].is_down = false;
    input_state.mouse_state.buttons[button].is_pressed = false;

    input_state.mouse_state.buttons[button].is_released = true;
  }

  else if (action == GLFW_PRESS) {
    input_state.mouse_state.buttons[button].is_down = true;
    input_state.mouse_state.buttons[button].is_pressed = true;

    input_state.mouse_state.buttons[button].is_released = false;
  }
}

static void cursor_pos_callback(GLFWwindow *window, double pos_x, double pos_y) {
  UNUSED(window);

  input_state.mouse_state.position.x = (float) pos_x;
  input_state.mouse_state.position.y = (float) pos_y;
}

int t_begin(int window_width, int window_height, const char* title) {
    printf("Initializing GLFW...\n");

    if (glfwInit() != GLFW_TRUE) {
      printf("Failed to initialize GLFW.\n");
      exit(EXIT_FAILURE);
    }

    printf("GLFW initialized succesfully.\n");

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    printf("Creating GLFW window...\n");
    s_window = glfwCreateWindow(window_width, window_height, title, NULL, NULL);

    if (!s_window) {
      printf("Failed to create a GLFW windo w.\n");
      glfwTerminate();
      exit(EXIT_FAILURE);
    }

    printf("GLFW window created successfuly.\n");

    glfwMakeContextCurrent(s_window);
    glfwSetCursorPosCallback(s_window, cursor_pos_callback);
    glfwSetMouseButtonCallback(s_window, mouse_button_callback);

    printf("Initializing GLAD...\n");

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
      printf("Failed to initialize GLAD.\n");
      glfwDestroyWindow(s_window);
      glfwTerminate();
      exit(EXIT_FAILURE);
    }

    printf("GLAD initialized successsfuly.\n");
    printf("OpenGL version: %s\n", glGetString(GL_VERSION));

    // GLint extensions_count;
    // glGetIntegerv(GL_NUM_EXTENSIONS, &extensions_count);

    // printf("OpenGL extensions:\n");
    // for (GLint i = 0; i < extensions_count; i++) {
    //     printf("%s\n", glGetStringi(GL_EXTENSIONS, i));
    // }

    #ifndef __APPLE__
      glDebugMessageCallback(debug_callback, NULL);
    #endif

    int width, height;
    glfwGetFramebufferSize(s_window, &width, &height);
    glViewport(0, 0, width, height);
    //const float ratio = width / (float)height;

    window_size = (t_vec2) { window_width, window_height };
    framebuffer_size = (t_vec2) { width, height };

    printf("window_size: %d, %d\n", window_width, window_height);
    printf("framebuffer_size: %d, %d\n", width, height);

    init_sprite_renderer();
    init_font_renderer();
    init_shapes_renderer();
    // BEGIN AUDIO

    printf("Initializing miniaudio...\n");

    ma_result result = ma_engine_init(NULL, &s_audio_engine);
    if (result != MA_SUCCESS) {
      printf("Problem initializing miniaudio: %d\n", result);
      return -1;
    }
    printf("miniaudio initialized successsfuly.\n");

    // END AUDIO

    glEnable(GL_STENCIL_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    //create miton blur framebuffer
    create_framebuffer();
    init_quad();

    motion_blur_shader_id = t_create_shader_program("./res/shaders/motion_blur_shader.vs", "./res/shaders/motion_blur_shader.fs");
    return 0;
}

bool t_loop() {
    calculate_delta_time();

    // bind_framebuffer(frame_buffer_id);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    return !glfwWindowShouldClose(s_window);
}

void t_draw_scene() {
    // bind_framebuffer(0);

    // glClearColor(0, 0, 0, 1);
    // glClear(GL_COLOR_BUFFER_BIT);

    // glUseProgram(motion_blur_shader_id);
    // glBindVertexArray(motion_blur_quad_vao);
    // glDisable(GL_DEPTH_TEST);

    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_2D, first_pass_texture_id);

    // glActiveTexture(GL_TEXTURE1);
    // glBindTexture(GL_TEXTURE_2D, motion_vector_texture_id);

    // glUniform1i(glGetUniformLocation(motion_blur_shader_id, "u_texture"), 0);
    // glUniform1i(glGetUniformLocation(motion_blur_shader_id, "u_motion_texture"), 1);

    // glDrawArrays(GL_TRIANGLES, 0, 6);
}

void t_loop_end() {

    // mat4_models_prev_index = 0;

    for (uint8_t i = 0; i < 2; i++) {
      input_state.mouse_state.buttons[i].is_pressed = false;
      input_state.mouse_state.buttons[i].is_released = false;
    }

    glfwSwapBuffers(s_window);
    glfwPollEvents();
}

void t_end() {
    terminate_font_renderer();
    glfwDestroyWindow(s_window);
    glfwTerminate();
}