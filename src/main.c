#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// GL loader
#include "../dep/include/glad/glad.h"
// Windowing
#include "../dep/include/GLFW/glfw3.h"

// MATH
#include "../dep/include/cglm/cglm.h"

#include "t_core.h"
#include "t_sprite.h"
#include "t_font.h"
#include "t_ui.h"

const unsigned int WINDOW_WIDTH = 640;
const unsigned int WINDOW_HEIGHT = 360;

float delta_time;
float last_frame_time;

float sprite_size_x = 48;
float sprite_size_y = 48;

t_global_state global_state;

bool left_mouse_button_pressed;

int clicked_count = 0;


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
  global_state.mouse_pos.x = pos_x;
   global_state.mouse_pos.y = pos_y;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    //reset state
    for (uint8_t i = 0; i < 2; i++) {
      global_state.input_state.mouse_state.buttons[i].is_pressed = false;
      global_state.input_state.mouse_state.buttons[i].is_released = false;
    } 

    //update state
    if (action == GLFW_RELEASE) {
      global_state.input_state.mouse_state.buttons[button].is_down = false;

      global_state.input_state.mouse_state.buttons[button].is_pressed = false;
      global_state.input_state.mouse_state.buttons[button].is_released = true;
    }
 
    else if(action == GLFW_PRESS) {
      global_state.input_state.mouse_state.buttons[button].is_down = true;
      global_state.input_state.mouse_state.buttons[button].is_pressed = true;

      global_state.input_state.mouse_state.buttons[button].is_released = false;
    }
}

void calculate_delta_time() {
  float current_frame_time = glfwGetTime();
  delta_time = current_frame_time - last_frame_time;
  last_frame_time = current_frame_time;
}

void on_button_clicked(t_ui_btn* button)
{
  clicked_count ++;
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
  sprite.texture = load_texture("./res/textures/panel-transparent-center-008.png");;
  sprite.slice_borders = (t_vec4){16, 16, 16, 16};
  sprite.scale = (t_vec2){1, 1};
  sprite.color = WHITE;
  sprite.texture_slice = (t_vec4){ 0, 0, 48, 48 };

  t_ui_btn button;
  button.sprite = &sprite;
  button.rect = (t_rect){width / 2 - 128 / 2, height / 2 - 48 / 2, 128, 48};
  button.color_default = WHITE;
  button.color_mouseover = RED;
  button.color_clicked = GREEN;
  button.on_clicked = on_button_clicked;

  while (!glfwWindowShouldClose(window)) {

    calculate_delta_time();
    //processInput(window);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(36.0 / 255, 10.0 / 255, 52.0 / 255, 1);

    //draw_sprite(&sprite, width / 2 - 128 / 2, height / 2 - 48 / 2, 128, 48);

    draw_ui_button(&button);
    char str[20]; // Assuming the string won't exceed 20 characters

    // Convert float to string
    sprintf_s(str, 20, "Clicks: %d", clicked_count);

    draw_text(str, (t_vec2){16, 16}, 18, WHITE);
    draw_text("Hey!", (t_vec2){16, 146}, 32, RED);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}
