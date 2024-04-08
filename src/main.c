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

#include "tailored.h"

const unsigned int WINDOW_WIDTH = 640;
const unsigned int WINDOW_HEIGHT = 360;

float delta_time;
float last_frame_time;

float sprite_size_x = 48;
float sprite_size_y = 48;

void processInput(GLFWwindow *window)
{
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

unsigned int quad_vao;
unsigned int sprite_shader;
unsigned int sprite_texture;

void init_quad() {
  // configure VAO/VBO
  unsigned int vertex_buffer_object;

  float vertices[] = {
      // pos      // tex
      0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,

      0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f};

  glGenVertexArrays(1, &quad_vao);
  glGenBuffers(1, &vertex_buffer_object);

  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glBindVertexArray(quad_vao);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void init_shader() {
  sprite_shader = create_shader_program("./res/shaders/shader.vs",
                                        "./res/shaders/shader.fs");

  sprite_texture = load_texture("./res/textures/panel-border-030.png")->id;
}

void draw_scene() {

  glUseProgram(sprite_shader);

  mat4 mat4_projection;
  glm_ortho(0, 640, 360, 0, 0.0, 1.0, mat4_projection);

  mat4 mat4_model;
  glm_mat4_identity(mat4_model);
  glm_translate(mat4_model,
                (vec3){640.0f / 2 - 24.0f, 360.0f / 2 - 24.0f, 0.0f});
  glm_scale(mat4_model, (vec3){sprite_size_x, sprite_size_y, 1.0f});

  glUniformMatrix4fv(glGetUniformLocation(sprite_shader, "u_mat4_projection"),
                     1, GL_FALSE, (float *)mat4_projection);

  glUniformMatrix4fv(glGetUniformLocation(sprite_shader, "u_mat4_model"), 1,
                     GL_FALSE, (float *)mat4_model);

  glUniform4fv(glGetUniformLocation(sprite_shader, "u_color"), 1,
               (vec4){1, 1, 1, 1.0f});

  glUniform4fv(glGetUniformLocation(sprite_shader, "u_slice_borders"), 1,
               (vec4){10.0f, 10.0f, 10.0f, 10.0f});

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, sprite_texture);

  glBindVertexArray(quad_vao);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindVertexArray(0);
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

  init_quad();
  init_shader();

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  while (!glfwWindowShouldClose(window)) {

    calculate_delta_time();
    processInput(window);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(36.0 / 255, 10.0 / 255, 52.0 / 255, 1);

    draw_scene();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}
