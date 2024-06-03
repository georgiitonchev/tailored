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

static float s_time = 0;
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

static t_rect s_clip_area;

void t_begin_scissor(int x, int y, int width, int height) {

  const t_vec2 window_size = t_window_size();
  const t_vec2 framebuffer_size = t_framebuffer_size();

  s_clip_area = (t_rect) { x, y, width, height };

  glEnable(GL_SCISSOR_TEST);
  glScissor(
      (x / window_size.x) * framebuffer_size.x,
      ((window_size.y - y - height) / window_size.y) * framebuffer_size.y,
      (width / window_size.x) * framebuffer_size.x,
      (height / window_size.y) * framebuffer_size.y);
}

void t_end_scissor() {

  s_clip_area = RECT_ZERO;
  glDisable(GL_SCISSOR_TEST);
}

t_rect t_clip_area() {
  return s_clip_area;
}

float t_delta_time() {
  return s_delta_time;
}

float t_time() {
    return s_time;
}

void t_play_audio(const char* path) {
  ma_engine_play_sound(&s_audio_engine, path, NULL);
}

ma_result t_init_sound(const char* path, ma_sound* sound) {

  ma_result result = ma_sound_init_from_file(&s_audio_engine, path, 0, NULL, NULL, sound);

  if (result != MA_SUCCESS) {
    printf("Could not initialize sound: %s", path);
  }

  return result;
}

void t_uninit_sound(ma_sound* sound) {
  ma_sound_uninit(sound);
}

void t_fade_out_sound(ma_sound* sound, int time) {
  ma_sound_set_fade_in_milliseconds(sound, -1, 0, time * 1000);
}

void t_fade_in_sound(ma_sound* sound, int time) {
  ma_sound_set_fade_in_milliseconds(sound, 0, 1, time * 1000);
}

void t_set_master_volume(float value) {
  ma_engine_set_volume(&s_audio_engine, value);
}

void t_start_sound(ma_sound* sound) {
  ma_sound_start(sound);
}

void t_set_cursor(const char* path, int xhot, int yhot) {

  t_texture_data texture_data = t_load_texture_data(path);

  GLFWimage image;
  image.width = texture_data.width;
  image.height = texture_data.height;
  image.pixels = texture_data.bytes;

  GLFWcursor* cursor = glfwCreateCursor(&image, xhot, yhot);
  glfwSetCursor(s_window, cursor);
 }

static void calculate_delta_time() {
  float current_frame_time = glfwGetTime();
  s_delta_time = current_frame_time - s_last_frame_time;
  s_last_frame_time = current_frame_time;

  s_time += s_delta_time;
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
    t_log_info("Initializing GLFW...");

    if (glfwInit() != GLFW_TRUE) {
      t_log_error("Failed to initialize GLFW.");
      exit(EXIT_FAILURE);
    }

    t_log_info("GLFW initialized succesfully.");

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    t_log_info("Creating GLFW window...");
    s_window = glfwCreateWindow(window_width, window_height, title, NULL, NULL);

    if (!s_window) {
      t_log_error("Failed to create a GLFW windo w.");
      glfwTerminate();
      exit(EXIT_FAILURE);
    }

    t_log_info("GLFW window created successfuly.");

    glfwMakeContextCurrent(s_window);
    glfwSetCursorPosCallback(s_window, cursor_pos_callback);
    glfwSetMouseButtonCallback(s_window, mouse_button_callback);
    glfwSwapInterval(0);

    t_log_info("Initializing GLAD...");

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
      t_log_error("Failed to initialize GLAD.");
      glfwDestroyWindow(s_window);
      glfwTerminate();
      exit(EXIT_FAILURE);
    }

    t_log_info("GLAD initialized successsfuly.");
    t_log_info("OpenGL version: %s", glGetString(GL_VERSION));

    #ifndef __APPLE__
      glDebugMessageCallback(debug_callback, NULL);
    #endif

    int width, height;
    glfwGetFramebufferSize(s_window, &width, &height);
    glViewport(0, 0, width, height);

    window_size = (t_vec2) { window_width, window_height };
    framebuffer_size = (t_vec2) { width, height };

    t_log_info("Window size: %d, %d", window_width, window_height);
    t_log_info("Framebuffer size: %d, %d", width, height);

    t_init_sprite_renderer();
    t_init_font_renderer();
    t_init_shapes_renderer();

    s_clip_area = RECT_ZERO;

    t_log_info("Initializing miniaudio...");

    ma_result result = ma_engine_init(NULL, &s_audio_engine);
    if (result != MA_SUCCESS) {
      t_log_error("Problem initializing miniaudio: %d", result);
      return -1;
    }
    t_log_info("miniaudio initialized successsfuly.");

    glEnable(GL_STENCIL_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return 0;
}

bool t_loop() {
    calculate_delta_time();

    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    return !glfwWindowShouldClose(s_window);
}

void t_loop_end() {

    for (uint8_t i = 0; i < 2; i++) {
      input_state.mouse_state.buttons[i].is_pressed = false;
      input_state.mouse_state.buttons[i].is_released = false;
    }

    glfwSwapBuffers(s_window);
    glfwPollEvents();
}

void t_end() {
    t_terminate_font_renderer();
    glfwDestroyWindow(s_window);
    glfwTerminate();
}
