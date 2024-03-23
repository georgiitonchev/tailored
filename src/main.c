#include <OpenGL/OpenGL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// GL loader
#include "../dep/include/glad/glad.h"
// Windowing
#include "../dep/include/GLFW/glfw3.h"
// STB
#define STB_IMAGE_IMPLEMENTATION
#include "../dep/include/stb/stb_image.h"
#define STB_DS_IMPLEMENTATION
#include "../dep/include/stb/stb_ds.h"
// MATH
#include "../dep/include/cglm/cglm.h"

typedef struct Vertex {
  vec3 position;
  vec3 normal;
  vec3 tex_coords;
} Vertex;

typedef struct Texture {
  unsigned int id;
  const char *type;
} Texture;

typedef struct Mesh {
  Vertex *vertices;
  unsigned int *indices;
  Texture *textures;

  unsigned int vertex_array_object;
  unsigned int vertex_buffer_object;
  unsigned int element_buffer_object;
} Mesh;

void setup_mesh(Mesh *mesh) {
  glGenVertexArrays(1, &mesh->vertex_array_object);
  glGenBuffers(1, &mesh->vertex_buffer_object);
  glGenBuffers(1, &mesh->element_buffer_object);

  glBindVertexArray(mesh->vertex_array_object);
  glBindBuffer(GL_ARRAY_BUFFER, mesh->vertex_buffer_object);

  glBufferData(GL_ARRAY_BUFFER, arrlen(mesh->vertices) * sizeof(Vertex),
               &mesh->vertices[0], GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->element_buffer_object);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               arrlen(mesh->indices) * sizeof(unsigned int), &mesh->indices[0],
               GL_STATIC_DRAW);

  // vertex positions
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
  // vertex normals
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, normal));
  // vertex texture coords
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, tex_coords));

  glBindVertexArray(0);
}

void draw_mesh(Mesh *mesh, unsigned int shader_program) {

  unsigned int diffuse_index = 1;
  unsigned int specular_index = 1;

  for (unsigned int i = 0; i < arrlen(mesh->textures); i++) {
    glActiveTexture(GL_TEXTURE0 + i);

    int index;
    const char *name = mesh->textures[i].type;
    if (strcmp(name, "texture_diffuse"))
      index = diffuse_index++;
    else if (strcmp(name, "texture_specular"))
      index = specular_index++;

    char buffer[100];
    int cx = snprintf(buffer, 100, "material.%s%d", name, index);

    glUniform1i(glGetUniformLocation(shader_program, buffer), i);
    glBindTexture(GL_TEXTURE_2D, mesh->textures[i].id);
  }

  glActiveTexture(GL_TEXTURE0);

  // draw mesh
  glBindVertexArray(mesh->vertex_array_object);
  glDrawElements(GL_TRIANGLES, arrlen(mesh->indices), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}

const unsigned int WINDOW_WIDTH = 640;
const unsigned int WINDOW_HEIGHT = 360;

void cursor_pos_callback(GLFWwindow *window, double xpos, double ypos);
void process_input(GLFWwindow *window);

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

  float vertices[] = {
      // positions          // normals           // texture coords
      -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f, 0.0f,  0.0f,  0.5f,  -0.5f,
      -0.5f, 0.0f,  0.0f,  -1.0f, 1.0f,  0.0f,  0.5f,  0.5f,  -0.5f, 0.0f,
      0.0f,  -1.0f, 1.0f,  1.0f,  0.5f,  0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f,
      1.0f,  1.0f,  -0.5f, 0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f, 0.0f,  1.0f,
      -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f, 0.0f,  0.0f,

      -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.5f,  -0.5f,
      0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,  0.5f,  0.5f,  0.5f,  0.0f,
      0.0f,  1.0f,  1.0f,  1.0f,  0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
      1.0f,  1.0f,  -0.5f, 0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
      -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

      -0.5f, 0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,  1.0f,  0.0f,  -0.5f, 0.5f,
      -0.5f, -1.0f, 0.0f,  0.0f,  1.0f,  1.0f,  -0.5f, -0.5f, -0.5f, -1.0f,
      0.0f,  0.0f,  0.0f,  1.0f,  -0.5f, -0.5f, -0.5f, -1.0f, 0.0f,  0.0f,
      0.0f,  1.0f,  -0.5f, -0.5f, 0.5f,  -1.0f, 0.0f,  0.0f,  0.0f,  0.0f,
      -0.5f, 0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,  1.0f,  0.0f,

      0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.5f,  0.5f,
      -0.5f, 1.0f,  0.0f,  0.0f,  1.0f,  1.0f,  0.5f,  -0.5f, -0.5f, 1.0f,
      0.0f,  0.0f,  0.0f,  1.0f,  0.5f,  -0.5f, -0.5f, 1.0f,  0.0f,  0.0f,
      0.0f,  1.0f,  0.5f,  -0.5f, 0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
      0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

      -0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,  0.0f,  1.0f,  0.5f,  -0.5f,
      -0.5f, 0.0f,  -1.0f, 0.0f,  1.0f,  1.0f,  0.5f,  -0.5f, 0.5f,  0.0f,
      -1.0f, 0.0f,  1.0f,  0.0f,  0.5f,  -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,
      1.0f,  0.0f,  -0.5f, -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,  0.0f,  0.0f,
      -0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,  0.0f,  1.0f,

      -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.5f,  0.5f,
      -0.5f, 0.0f,  1.0f,  0.0f,  1.0f,  1.0f,  0.5f,  0.5f,  0.5f,  0.0f,
      1.0f,  0.0f,  1.0f,  0.0f,  0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
      1.0f,  0.0f,  -0.5f, 0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
      -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,  0.0f,  1.0f};

  unsigned int vertex_buffer_object, vertex_array_object, element_array_object;
  glGenVertexArrays(1, &vertex_array_object);
  glGenBuffers(1, &vertex_buffer_object);
  // bind the Vertex Array Object first, then bind and set vertex buffer(s), and
  // then configure vertex attributes(s).
  glBindVertexArray(vertex_array_object);

  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, 8 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
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

unsigned int load_texture(const char *texture_path, int texture_format) {
  int width, height, channels;
  unsigned char *data = stbi_load(texture_path, &width, &height, &channels, 0);

  unsigned int texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, texture_format, width, height, 0,
               texture_format, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);

  stbi_image_free(data);

  return texture;
}

vec3 cam_pos = {0, 0, 3};
vec3 cam_dir = {0, 0, -1};
vec3 cam_up = {0, 1, 0};

vec3 light_pos = {1.2f, 1.0f, 2.0f};

float delta_time;
float last_frame_time;

float mouse_last_x;
float mouse_last_y;

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
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  printf("Creating GLFW window...\n");
  GLFWwindow *window =
      glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Tailored", NULL, NULL);

  if (!window) {
    printf("Failed to create a GLFW window.\n");
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  printf("GLFW window created successfuly.\n");

  glfwMakeContextCurrent(window);
  // glfwSetCursorPosCallback(window, cursor_pos_callback);
  // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

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

  unsigned int shader_program = create_shader_program();
  unsigned int vertex_array_object = create_vertex_array_object();

  stbi_set_flip_vertically_on_load(1);
  unsigned int u_texture =
      load_texture("./res/textures/container2.png", GL_RGBA);

  unsigned int u_texture_specular =
      load_texture("./res/textures/container2_specular.png", GL_RGBA);

  glUseProgram(shader_program);
  glUniform1i(glGetUniformLocation(shader_program, "u_texture"), 0);
  glUniform1i(glGetUniformLocation(shader_program, "u_texture_specular"), 1);

  glUniform3fv(glGetUniformLocation(shader_program, "u_object_color"), 1,
               (vec3){1, 0.5f, 0.3f});

  glUniform3fv(glGetUniformLocation(shader_program, "u_dir_light.color"), 1,
               (vec3){1, 1, 1});
  glUniform3fv(glGetUniformLocation(shader_program, "u_dir_light.direction"), 1,
               (vec3){-1, 0, 0});
  glUniform3fv(glGetUniformLocation(shader_program, "u_point_light.color"), 1,
               (vec3){1, 1, 1});

  glUniform1f(glGetUniformLocation(shader_program, "u_point_light.constant"),
              1.0f);
  glUniform1f(glGetUniformLocation(shader_program, "u_point_light.linear"),
              0.09f);
  glUniform1f(glGetUniformLocation(shader_program, "u_point_light.quadratic"),
              0.032f);

  glUniform1f(glGetUniformLocation(shader_program, "u_spot_light.constant"),
              1.0f);
  glUniform1f(glGetUniformLocation(shader_program, "u_spot_light.linear"),
              0.09f);
  glUniform1f(glGetUniformLocation(shader_program, "u_spot_light.quadratic"),
              0.032f);

  glUniform3fv(glGetUniformLocation(shader_program, "u_spot_light.color"), 1,
               (vec3){1, 1, 1});

  glUniform3fv(glGetUniformLocation(shader_program, "u_spot_light.direction"),
               1, cam_dir);

  glUniform1f(glGetUniformLocation(shader_program, "u_spot_light.inner_angle"),
              cos(glm_rad(8.0)));

  glUniform1f(glGetUniformLocation(shader_program, "u_spot_light.outer_angle"),
              cos(glm_rad(9.0)));

  glEnable(GL_DEPTH_TEST);
  while (!glfwWindowShouldClose(window)) {

    glUniform3fv(glGetUniformLocation(shader_program, "u_spot_light.position"),
                 1, cam_pos);

    glUniform3fv(glGetUniformLocation(shader_program, "u_point_light.position"),
                 1, cam_pos);

    float current_frame_time = glfwGetTime();
    delta_time = current_frame_time - last_frame_time;
    last_frame_time = current_frame_time;

    process_input(window);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(36.0 / 255, 10.0 / 255, 52.0 / 255, 1);

    // identity matrix
    mat4 mat_view = {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}};
    glm_look(cam_pos, cam_dir, cam_up, mat_view);

    mat4 mat_projection;
    glm_perspective(glm_rad(45.0f), ratio, .1f, 100.0f, mat_projection);

    mat4 mat_model;
    glm_mat4_identity(mat_model);
    glm_rotate(mat_model, (float)glfwGetTime(), (vec3){0, 0.6f, 0.3f});
    glUniformMatrix4fv(glGetUniformLocation(shader_program, "u_model"), 1,
                       GL_FALSE, (float *)mat_model);

    glUniformMatrix4fv(glGetUniformLocation(shader_program, "u_view"), 1,
                       GL_FALSE, (float *)mat_view);
    glUniformMatrix4fv(glGetUniformLocation(shader_program, "u_projection"), 1,
                       GL_FALSE, (float *)mat_projection);

    glUniform3fv(glGetUniformLocation(shader_program, "u_cam_pos"), 1, cam_pos);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, u_texture);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, u_texture_specular);

    glBindVertexArray(vertex_array_object);
    glDrawArrays(GL_TRIANGLES, 0, 36);

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

void process_input(GLFWwindow *window) {
  const float cam_speed = 2.5f * delta_time;
  vec3 vel = {0, 0, 0};
  vec3 cam_right = {0, 0, 0};

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    glm_vec3_scale(cam_dir, cam_speed, vel);
    glm_vec3_add(cam_pos, vel, cam_pos);
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    glm_vec3_scale(cam_dir, -cam_speed, vel);
    glm_vec3_add(cam_pos, vel, cam_pos);
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    glm_vec3_cross(cam_dir, cam_up, cam_right);
    glm_vec3_normalize(cam_right);
    glm_vec3_scale(cam_right, -cam_speed, vel);
    glm_vec3_add(cam_pos, vel, cam_pos);
  }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    glm_vec3_cross(cam_dir, cam_up, cam_right);
    glm_vec3_normalize(cam_right);
    glm_vec3_scale(cam_right, cam_speed, vel);
    glm_vec3_add(cam_pos, vel, cam_pos);
  }

  glm_vec3(cam_pos, light_pos);
}

float yaw;
float pitch;

void cursor_pos_callback(GLFWwindow *window, double xposIn, double yposIn) {
  float xpos = (float)xposIn;
  float ypos = (float)yposIn;

  float xoffset = xpos - mouse_last_x;
  float yoffset =
      mouse_last_y - ypos; // reversed since y-coordinates go from bottom to top
  mouse_last_x = xpos;
  mouse_last_y = ypos;

  float sensitivity = 0.1f; // change this value to your liking
  xoffset *= sensitivity;
  yoffset *= sensitivity;

  yaw += xoffset;
  pitch += yoffset;

  // make sure that when pitch is out of bounds, screen doesn't get flipped
  if (pitch > 89.0f)
    pitch = 89.0f;
  if (pitch < -89.0f)
    pitch = -89.0f;

  cam_dir[0] = cos(glm_rad(yaw)) * cos(glm_rad(pitch));
  cam_dir[1] = sin(glm_rad(pitch));
  cam_dir[2] = sin(glm_rad(yaw)) * cos(glm_rad(pitch));
  glm_normalize(cam_dir);
}
