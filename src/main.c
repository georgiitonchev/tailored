#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// GL loader
#include "../dep/include/glad/glad.h"
// Windowing
#include "../dep/include/GLFW/glfw3.h"

// MATH
#include "../dep/include/cglm/cglm.h"

#include "t_sprite.h"
#include "tailored.h"

const unsigned int WINDOW_WIDTH = 640;
const unsigned int WINDOW_HEIGHT = 360;

float delta_time;
float last_frame_time;

float sprite_size_x = 48;
float sprite_size_y = 48;

void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    sprite_size_y++;
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    sprite_size_y--;
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    sprite_size_x--;
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    sprite_size_x++;
}

void calculate_delta_time() {
  float current_frame_time = glfwGetTime();
  delta_time = current_frame_time - last_frame_time;
  last_frame_time = current_frame_time;
}

int main() {
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

  printf("Creating GLFW window...\n");
  GLFWwindow *window =
      glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Tailored", NULL, NULL);

  if (!window) {
    printf("Failed to create a GLFW windo w.\n");
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  printf("GLFW window created successfuly.\n");

  glfwMakeContextCurrent(window);

  printf("Initializing GLAD...\n");

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    printf("Failed to initialize GLAD.\n");
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  printf("GLAD initialized successsfuly.\n");

  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  glViewport(0, 0, width, height);
  const float ratio = width / (float)height;

  init_sprite_renderer();

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  t_sprite sprite;
  sprite.textue =
      load_texture("./res/textures/panel-transparent-center-008.png");
  sprite.slice_borders = (t_vec4){16, 16, 16, 16};
  sprite.scale = (t_vec2){1, 1};

  t_transform sprite_transform;
  sprite_transform.position = (t_vec3){100, 100, 0};
  sprite_transform.size = (t_vec3){64, 64, 1};

  t_sprite sprite_b;
  sprite_b.textue = sprite.textue;
  sprite_b.slice_borders = (t_vec4){16, 16, 16, 16};
  sprite_b.scale = (t_vec2){1, 1};

  t_transform sprite_b_transform;
  sprite_b_transform.position = (t_vec3){320, 100, 0};
  sprite_b_transform.size = (t_vec3){128, 48, 1};

  while (!glfwWindowShouldClose(window)) {

    calculate_delta_time();
    processInput(window);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(36.0 / 255, 10.0 / 255, 52.0 / 255, 1);

    draw_sprite(&sprite, sprite_transform);
    draw_sprite(&sprite_b, sprite_b_transform);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}
