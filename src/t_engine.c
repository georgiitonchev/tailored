#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#endif

#include "t_engine.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Audio
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

// GL loader
#include "../dep/include/glad/glad.h"
// Windowing
#include "../dep/include/GLFW/glfw3.h"
// MATH
#include "../dep/include/cglm/cglm.h"

#include "t_core.h"
#include "t_sprite.h"
#include "t_font.h"

static GLFWwindow* m_window;
static ma_engine m_engine;

static float delta_time;
static float last_frame_time;

t_global_state global_state;

void t_play_audio(const char* path) {
  ma_engine_play_sound(&m_engine, path, NULL);
}

void t_set_cursor(const char* path, int xhot, int yhot) {

  t_texture_data texture_data = load_texture_data(path);

  GLFWimage image;
  image.width = texture_data.width;
  image.height = texture_data.height;
  image.pixels = texture_data.data;

  GLFWcursor* cursor = glfwCreateCursor(&image, xhot, yhot);
  glfwSetCursor(m_window, cursor);
 }

static void calculate_delta_time() {
  float current_frame_time = glfwGetTime();
  delta_time = current_frame_time - last_frame_time;
  last_frame_time = current_frame_time;

  global_state.delta_time = delta_time;
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
    global_state.input_state.mouse_state.buttons[button].is_down = false;
    global_state.input_state.mouse_state.buttons[button].is_pressed = false;

    global_state.input_state.mouse_state.buttons[button].is_released = true;
  }

  else if (action == GLFW_PRESS) {
    global_state.input_state.mouse_state.buttons[button].is_down = true;
    global_state.input_state.mouse_state.buttons[button].is_pressed = true;

    global_state.input_state.mouse_state.buttons[button].is_released = false;
  }
}

static void cursor_pos_callback(GLFWwindow *window, double pos_x, double pos_y) {
  UNUSED(window);

  global_state.mouse_pos.x = (float) pos_x;
  global_state.mouse_pos.y = (float) pos_y;
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
    m_window = glfwCreateWindow(window_width, window_height, title, NULL, NULL);

    if (!m_window) {
      printf("Failed to create a GLFW windo w.\n");
      glfwTerminate();
      exit(EXIT_FAILURE);
    }

    printf("GLFW window created successfuly.\n");

    glfwMakeContextCurrent(m_window);
    glfwSetCursorPosCallback(m_window, cursor_pos_callback);
    glfwSetMouseButtonCallback(m_window, mouse_button_callback);

    printf("Initializing GLAD...\n");

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
      printf("Failed to initialize GLAD.\n");
      glfwDestroyWindow(m_window);
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
    glfwGetFramebufferSize(m_window, &width, &height);
    glViewport(0, 0, width, height);
    //const float ratio = width / (float)height;

    global_state.window_size = (t_vec2) { window_width, window_height };
    global_state.framebuffer_size = (t_vec2) { width, height };

    printf("window_size: %d, %d\n", window_width, window_height);
    printf("framebuffer_size: %d, %d\n", width, height);

    init_sprite_renderer();
    init_font_renderer();

    // BEGIN AUDIO

    printf("Initializing miniaudio...\n");

    ma_result result = ma_engine_init(NULL, &m_engine);
    if (result != MA_SUCCESS) {
      printf("Problem initializing miniaudio: %d\n", result);
      return -1;
    }
    printf("miniaudio initialized successsfuly.\n");

    // END AUDIO

    //glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return 0;
}

bool t_loop() {
    calculate_delta_time();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    return !glfwWindowShouldClose(m_window);
}

void t_loop_end() {

    for (uint8_t i = 0; i < 2; i++) {
      global_state.input_state.mouse_state.buttons[i].is_pressed = false;
      global_state.input_state.mouse_state.buttons[i].is_released = false;
    }

    glfwSwapBuffers(m_window);
    glfwPollEvents();
}

void t_end() {
    terminate_font_renderer();
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void t_begin_scissor(int x, int y, int width, int height) {
  glEnable(GL_SCISSOR_TEST);
  glScissor(
      (x / global_state.window_size.x) * global_state.framebuffer_size.x,
      ((global_state.window_size.y - y - height) / global_state.window_size.y) * global_state.framebuffer_size.y,
      (width / global_state.window_size.x) * global_state.framebuffer_size.x,
      (height / global_state.window_size.y) * global_state.framebuffer_size.y);
}

void t_end_scissor() {
  glDisable(GL_SCISSOR_TEST);
}
