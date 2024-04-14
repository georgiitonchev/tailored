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

#include "t_core.h"
#include "t_sprite.h"
#include "t_font.h"

const unsigned int WINDOW_WIDTH = 640;
const unsigned int WINDOW_HEIGHT = 360;

float delta_time;
float last_frame_time;

float sprite_size_x = 48;
float sprite_size_y = 48;

t_vec2 mouse_position;

bool left_mouse_button_pressed;

void process_input(GLFWwindow *window) {
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

void cursor_pos_callback(GLFWwindow* window, double pos_x, double pos_y) {
  mouse_position.x = pos_x;
  mouse_position.y = pos_y;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
          left_mouse_button_pressed = true;
        if (action == GLFW_RELEASE)
          left_mouse_button_pressed = false;
    }
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
  glfwSetCursorPosCallback(window, cursor_pos_callback);
  glfwSetMouseButtonCallback(window, mouse_button_callback);

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
  init_font_renderer();

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  t_sprite sprite;
  sprite.texture =
      load_texture("./res/textures/panel-transparent-center-008.png");
  sprite.slice_borders = (t_vec4){0, 0, 0, 0};
  sprite.scale = (t_vec2){1, 1};
  sprite.color = (t_vec4){1, 1, 1, 1};
  sprite.texture_slice = (t_vec4){2, 2, 24, 24};

  t_sprite sprite_b;
  sprite_b.texture = sprite.texture;
  sprite_b.slice_borders = (t_vec4){16, 16, 16, 16};
  sprite_b.scale = (t_vec2){1, 1};
  sprite_b.color = (t_vec4){1, 1, 1, 1};
  sprite_b.texture_slice = (t_vec4){ 0, 0, 48, 48 };

  while (!glfwWindowShouldClose(window)) {

    calculate_delta_time();
    //processInput(window);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(36.0 / 255, 10.0 / 255, 52.0 / 255, 1);

    draw_sprite(&sprite, (t_vec2){64, 64}, (t_vec2){64, 64});
    draw_sprite(&sprite_b, (t_vec2){164, 64}, (t_vec2){128, 48});

    if (mouse_position.x >= 164 && mouse_position.x <= 164 + 128 && mouse_position.y >= 64 && mouse_position.y <= 64 + 48) {
      if (left_mouse_button_pressed)
        sprite_b.color = (t_vec4){.33f, .33f, .33f, 1};
      else
        sprite_b.color = (t_vec4){.66f, .66f, .66f, 1};
    }
    else
      sprite_b.color = (t_vec4){1, 1, 1, 1};

    char str[20]; // Assuming the string won't exceed 20 characters

    // Convert float to string
    sprintf_s(str, 20, "FPS: %d", (int)((1 / delta_time) + .5f));

    draw_text(str, (t_vec2){16, 16}, 18, WHITE);
    draw_text("Hey!", (t_vec2){16, 146}, 32, RED);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}
