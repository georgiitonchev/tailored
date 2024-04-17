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
#include "t_font.h"
#include "t_sprite.h"
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

ma_result result;
ma_engine engine;

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

void cursor_pos_callback(GLFWwindow *window, double pos_x, double pos_y) {
  global_state.mouse_pos.x = pos_x;
  global_state.mouse_pos.y = pos_y;
}

void mouse_button_callback(GLFWwindow *window, int button, int action,
                           int mods) {
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

void calculate_delta_time() {
  float current_frame_time = glfwGetTime();
  delta_time = current_frame_time - last_frame_time;
  last_frame_time = current_frame_time;
}

void on_button_start_clicked(t_ui_btn *button) {
  clicked_count++;
  ma_engine_play_sound(&engine, "./res/audio/click_003.wav", NULL);
}

void on_button_quit_clicked(t_ui_btn *button) {
  clicked_count--;
  ma_engine_play_sound(&engine, "./res/audio/click_003.wav", NULL);
}

void on_button_mouse_enter() {
  ma_engine_play_sound(&engine, "./res/audio/click_004.wav", NULL);
}

void on_button_mouse_exit() {
  //ma_engine_play_sound(&engine, "./res/audio/click_004.wav", NULL);
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

  // BEGIN AUDIO

  printf("Initializing miniaudio...\n");

  result = ma_engine_init(NULL, &engine);
  if (result != MA_SUCCESS) {
    return -1;
  }
  printf("miniaudio initialized successsfuly.\n");

  // END AUDIO

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  t_sprite sprite;
  sprite.texture =
      load_texture("./res/textures/panel-transparent-center-008.png");
  
  sprite.slice_borders = (t_vec4){16, 16, 16, 16};
  sprite.scale = (t_vec2){1, 1};
  sprite.color = WHITE;
  sprite.texture_slice = (t_vec4){0, 0, 48, 48};

  t_ui_btn button_start = create_ui_button(&sprite, (t_rect){WINDOW_WIDTH / 2 - 128 / 2, (WINDOW_HEIGHT / 2 - 48 / 2) - 32, 128, 48});
  button_start.on_released = on_button_start_clicked;
  button_start.on_mouse_enter = on_button_mouse_enter;
  button_start.on_mouse_exit = on_button_mouse_exit;

  t_ui_btn button_quit = create_ui_button(&sprite, (t_rect){WINDOW_WIDTH / 2 - 128 / 2, (WINDOW_HEIGHT / 2 - 48 / 2) + 32, 128, 48});
  button_quit.on_released = on_button_quit_clicked;
  button_quit.on_mouse_enter = on_button_mouse_enter;
  button_quit.on_mouse_exit = on_button_mouse_exit;

  while (!glfwWindowShouldClose(window)) {

    calculate_delta_time();
    // processInput(window);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(36.0 / 255, 10.0 / 255, 52.0 / 255, 1);

    char str[20]; // Assuming the string won't exceed 20 characters

    // Convert float to string
    sprintf(str, "Clicks: %d", clicked_count);
    draw_text(str, (t_vec2){16, 16}, 18, WHITE);

    draw_ui_button(&button_start);
    draw_text("START", (t_vec2){ button_start.rect.x + button_start.rect.width / 2 - strlen("START") * 8, button_start.rect.y + button_start.rect.height / 2 - 8}, 16, button_start.sprite->color);

    draw_ui_button(&button_quit);
    draw_text("QUIT", (t_vec2){ button_quit.rect.x + button_quit.rect.width / 2 - strlen("QUIT") * 8, button_quit.rect.y + button_quit.rect.height / 2 - 8}, 16, button_quit.sprite->color);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}
