#include <stdio.h>
#include <stdlib.h>

#include "../dep/include/glad/glad.h"

#define GLFW_INCLUDE_NONE
#include "../dep/include/GLFW/glfw3.h"

const char *read_shader_file(const char *path) {
  char *vertex_shader_text;

  FILE *file_pointer = fopen(path, "rb");
  long file_size;

  if (file_pointer == NULL) {
    perror(path);
    exit(EXIT_FAILURE);
  }

  fseek(file_pointer, 0, SEEK_END);
  file_size = ftell(file_pointer);
  rewind(file_pointer);

  vertex_shader_text = (char *)malloc(file_size * sizeof(char));

  // Read file contents into the buffer
  fread(vertex_shader_text, sizeof(char), file_size, file_pointer);

  // Add null terminator at the end to make it a valid C string
  vertex_shader_text[file_size] = '\0';

  fclose(file_pointer);

  // Free allocated memory
  return vertex_shader_text;
}

int main() {
  printf("Hello, World!\n");
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

  printf("Creating GLFW window...\n");
  GLFWwindow *window = glfwCreateWindow(640, 360, "Tailored", NULL, NULL);

  if (!window) {
    printf("Failed to create a GLFW window.\n");
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

  while (!glfwWindowShouldClose(window)) {
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(36.0 / 255, 10.0 / 255, 52.0 / 255, 1);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}
