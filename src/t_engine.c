#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#endif

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

static float delta_time;
static float last_frame_time;

t_global_state global_state;

static void calculate_delta_time() {
  float current_frame_time = glfwGetTime();
  delta_time = current_frame_time - last_frame_time;
  last_frame_time = current_frame_time;
}

static void APIENTRY debug_callback(GLenum source, GLenum type, GLuint id,
  GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
  printf("OpenGL Debug: %s\n", message);
}

static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
  UNUSED(window);
  UNUSED(mods);

  // reset state
  for (uint8_t i = 0; i < 2; i++) {
    global_state.input_state.mouse_state.buttons[i].is_pressed = false;
    global_state.input_state.mouse_state.buttons[i].is_released = false;
  }

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

  global_state.mouse_pos.x = pos_x;
  global_state.mouse_pos.y = pos_y;
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

    GLint extensions_count;
    glGetIntegerv(GL_NUM_EXTENSIONS, &extensions_count);

    printf("OpenGL extensions:\n");
    for (GLint i = 0; i < extensions_count; i++) {
        printf("%s\n", glGetStringi(GL_EXTENSIONS, i));
    }

    #ifndef __APPLE__
      glDebugMessageCallback(debug_callback, NULL);
    #endif

    int width, height;
    glfwGetFramebufferSize(m_window, &width, &height);
    glViewport(0, 0, width, height);
    //const float ratio = width / (float)height;

    init_sprite_renderer();
    init_font_renderer();

    // BEGIN AUDIO

    printf("Initializing miniaudio...\n");

    ma_engine engine;
    ma_result result = ma_engine_init(NULL, &engine);
    if (result != MA_SUCCESS) {
      return -1;
    }
    printf("miniaudio initialized successsfuly.\n");

    // END AUDIO

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return 0;
}

bool t_loop() {

    calculate_delta_time();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(36.0 / 255, 10.0 / 255, 52.0 / 255, 1);

    glfwSwapBuffers(m_window);
    glfwPollEvents();

    return !glfwWindowShouldClose(m_window);
}

void t_end() {
    glfwDestroyWindow(m_window);
    glfwTerminate();
}
