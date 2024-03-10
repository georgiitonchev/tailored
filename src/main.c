#include <OpenGL/OpenGL.h>
#include <stdio.h>
#include <stdlib.h>

// GL loader
#include "../dep/include/glad/glad.h"
// Windowing
#include "../dep/include/GLFW/glfw3.h"
// STB
#define STB_IMAGE_IMPLEMENTATION
#include "../dep/include/stb/stb_image.h"
// MATH
#include "../dep/include/cglm/cglm.h"

const unsigned int WINDOW_WIDTH = 640;
const unsigned int WINDOW_HEIGHT = 360;

const char *read_file(const char *path) {
  char *text_buffer;

  FILE *file_pointer = fopen(path, "rb");
  long file_size;

  if (file_pointer == NULL) {
    perror(path);
    exit(EXIT_FAILURE);
  }

  fseek(file_pointer, 0, SEEK_END);
  file_size = ftell(file_pointer);
  rewind(file_pointer);

  text_buffer = (char *)malloc(file_size * sizeof(char));

  // Read file contents into the buffer
  fread(text_buffer, sizeof(char), file_size, file_pointer);

  // Add null terminator at the end to make it a valid C string
  text_buffer[file_size] = '\0';

  fclose(file_pointer);

  // Free allocated memory
  return text_buffer;
}

unsigned int create_shader_program() {

  // load vertex shader source
  const char *vertex_shader_source = read_file("./res/shaders/shader.vs");

  unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
  glCompileShader(vertex_shader);

  free((void *)vertex_shader_source);

  // check for shader compile errors
  int success;
  char info_log[512];
  glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertex_shader, 512, NULL, info_log);
    printf("%s", info_log);
  }

  const char *fragment_shader_source = read_file("./res/shaders/shader.fs");

  // fragment shader
  unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
  glCompileShader(fragment_shader);

  free((void *)fragment_shader_source);

  // check for shader compile errors
  glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragment_shader, 512, NULL, info_log);
    printf("%s", info_log);
  }
  // link shaders
  unsigned int shader_program = glCreateProgram();
  glAttachShader(shader_program, vertex_shader);
  glAttachShader(shader_program, fragment_shader);
  glLinkProgram(shader_program);
  // check for linking errors
  glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(shader_program, 512, NULL, info_log);
    printf("%s", info_log);
  }

  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  return shader_program;
}

unsigned int create_vertex_array_object() {
  // set up vertex data (and buffer(s)) and configure vertex attributes
  // ------------------------------------------------------------------
  float vertices[] = {
      // positions          // colors           // texture coords
      0.5f,  0.5f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top right
      0.5f,  -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom right
      -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom left
      -0.5f, 0.5f,  0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f  // top left
  };

  unsigned int indices[] = {
      // note that we start from 0!
      0, 1, 3, // first Triangle
      1, 2, 3  // second Triangle
  };
  unsigned int vertex_buffer_object, vertex_array_object, element_array_object;
  glGenVertexArrays(1, &vertex_array_object);
  glGenBuffers(1, &vertex_buffer_object);
  glGenBuffers(1, &element_array_object);
  // bind the Vertex Array Object first, then bind and set vertex buffer(s), and
  // then configure vertex attributes(s).
  glBindVertexArray(vertex_array_object);

  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_array_object);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
               GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, 8 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glVertexAttribPointer(2, 2, GL_FLOAT, GL_TRUE, 8 * sizeof(float),
                        (void *)(6 * sizeof(float)));
  glEnableVertexAttribArray(2);
  // note that this is allowed, the call to glVertexAttribPointer registered VBO
  // as the vertex attribute's bound vertex buffer object so afterwards we can
  // safely unbind
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // remember: do NOT unbind the EBO while a VAO is active as the bound element
  // buffer object IS stored in the VAO; keep the EBO bound.
  // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  // You can unbind the VAO afterwards so other VAO calls won't accidentally
  // modify this VAO, but this rarely happens. Modifying other VAOs requires a
  // call to glBindVertexArray anyways so we generally don't unbind VAOs (nor
  // VBOs) when it's not directly necessary.
  glBindVertexArray(0);
  return vertex_array_object;
}

unsigned int create_texture_a() {
  int width, height, channels;
  unsigned char *data =
      stbi_load("./res/textures/container.jpg", &width, &height, &channels, 0);

  unsigned int texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
               GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);

  stbi_image_free(data);

  return texture;
}

unsigned int create_texture_b() {
  int width, height, channels;
  unsigned char *data = stbi_load("./res/textures/awesomeface.png", &width,
                                  &height, &channels, 0);
  unsigned int texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  if (data) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
  }

  stbi_image_free(data);

  return texture;
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
  GLFWwindow *window =
      glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Tailored", NULL, NULL);

  mat4 m = {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}};

  glm_translate(m, (vec3){1.0f, 0.0f, 0.0f});

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

  unsigned int shader_program = create_shader_program();
  unsigned int vertex_array_object = create_vertex_array_object();

  stbi_set_flip_vertically_on_load(1);
  unsigned int texture_a = create_texture_a();
  unsigned int texture_b = create_texture_b();

  glUseProgram(shader_program);
  glUniform1i(glGetUniformLocation(shader_program, "u_texture_a"), 0);
  glUniform1i(glGetUniformLocation(shader_program, "u_texture_b"), 1);

  while (!glfwWindowShouldClose(window)) {
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(36.0 / 255, 10.0 / 255, 52.0 / 255, 1);

    // draw our first triangle

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_a);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture_b);

    glBindVertexArray(vertex_array_object);
    // glDrawArrays(GL_TRIANGLES, 0, 3); //also works, but ignores elements
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glDeleteVertexArrays(1, &vertex_array_object);
  // glDeleteBuffers(1, &VBO);
  // glDeleteBuffers(1, &EBO);
  glDeleteProgram(shader_program);

  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}
