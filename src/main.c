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

void cursor_pos_callback(GLFWwindow *window, double xpos, double ypos);
void process_input(GLFWwindow *window);


vec3 cam_pos = {0, 0, 3};
vec3 cam_dir = {0, 0, -1};
vec3 cam_up = {0, 1, 0};

vec3 light_pos = {1.2f, 1.0f, 2.0f};

float delta_time;
float last_frame_time;

float mouse_last_x;
float mouse_last_y;

bool rotate_model;

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

  t_scene *scenes = NULL;
  // process_gltf_file("./res/models/simple/scene.gltf", &model); // V
  // process_gltf_file("./res/models/triangle/Triangle.gltf", &model); // V
  // process_gltf_file("./res/models/triangle_without_indices/triangleWithoutIndices.gltf",
  // &model); // V
  // process_gltf_file("./res/models/cube/Cube.gltf", &scenes);
  // process_gltf_file("./res/models/avocado/Avocado.gltf", &scenes);
  // process_gltf_file("./res/models/corset/Corset.gltf", &model);
  // &model);
  // process_gltf_file("./res/models/simple_meshes/SimpleMeshes.gltf", &scenes);
  process_gltf_file("./res/scenes/scene_1/scene_4.gltf", &scenes);
  //process_gltf_file("./res/scenes/scene_2/scene_3.gltf", &scenes);

  if (scenes == NULL) {
    printf("Could not load scenes.\n");
  } else
    printf("Scenes loaded successfuly.\n");

  t_scene scene = scenes[0];
  for (unsigned int i = 0; i < scene.nodes_count; i++) {
    setup_mesh(&scene.nodes[i].mesh);
    printf("indices: %d\n", scene.nodes[i].mesh.indices_count);
    printf("vertices: %d\n", scene.nodes[i].mesh.vertices_count);
  }

  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  glViewport(0, 0, width, height);
  const float ratio = width / (float)height;

  unsigned int shader_program = create_shader_program("./res/shaders/shader.vs", "./res/shaders/shader.fs");

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_STENCIL_TEST);
  glEnable(GL_CULL_FACE);

  while (!glfwWindowShouldClose(window)) {

    float current_frame_time = glfwGetTime();
    delta_time = current_frame_time - last_frame_time;
    last_frame_time = current_frame_time;

    process_input(window);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glClearColor(36.0 / 255, 10.0 / 255, 52.0 / 255, 1);

    // identity matrix
    mat4 mat_view = {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}};
    glm_look(cam_pos, cam_dir, cam_up, mat_view);

    mat4 mat_projection;
    glm_perspective(glm_rad(45.0f), ratio, .1f, 100.0f, mat_projection);

    mat4 mat_model;
    glm_mat4_identity(mat_model);

    glUniformMatrix4fv(glGetUniformLocation(shader_program, "u_view"), 1,
                       GL_FALSE, (float *)mat_view);

    glUniformMatrix4fv(glGetUniformLocation(shader_program, "u_projection"), 1,
                       GL_FALSE, (float *)mat_projection);

    for (unsigned int i = 0; i < scene.nodes_count; i++) {

      t_node node = scene.nodes[i];

      glm_scale(mat_model,
                (vec3){node.transform.scale.x, node.transform.scale.y,
                       node.transform.scale.z});

      if (rotate_model)
        glm_rotate(mat_model, (float)glfwGetTime(), (vec3){0, 0.6f, 0.3f});

      glm_translate(mat_model,
                    (vec3){node.transform.position.x, node.transform.position.y,
                           node.transform.position.z});

      glUniformMatrix4fv(glGetUniformLocation(shader_program, "u_model"), 1,
                         GL_FALSE, (float *)mat_model);

      draw_mesh(&node.mesh, shader_program);
    }

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // glDeleteVertexArrays(1, &vertex_array_object);
  //  glDeleteBuffers(1, &VBO);
  //  glDeleteBuffers(1, &EBO);
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

  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
    rotate_model = !rotate_model;
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
