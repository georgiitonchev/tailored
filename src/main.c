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

void calculate_delta_time();

vec3 cam_pos = {0, 5, 5};
vec3 cam_dir = {0, -0.45, -1};
vec3 cam_up = {0, 1, 0};

vec3 light_pos = {0, 5, 5};

float delta_time;
float last_frame_time;

float mouse_last_x;
float mouse_last_y;

bool rotate_model;
bool use_blinn;

void calculate_delta_time()
{
  float current_frame_time = glfwGetTime();
  delta_time = current_frame_time - last_frame_time;
  last_frame_time = current_frame_time;
}

void render_scene(t_scene* scene, unsigned int program) {
  glUseProgram(program);

  for (unsigned int i = 0; i < scene->nodes_count; i++) {

      mat4 mat_model;
      glm_mat4_identity(mat_model);

      t_node node = scene->nodes[i];

      glm_quat_rotate_at(mat_model, 
        (vec4) {
          node.transform.rotation.x, 
          node.transform.rotation.y, 
          node.transform.rotation.z, 
          node.transform.rotation.w }, 
        (vec3) {node.transform.position.x, node.transform.position.y,
                           node.transform.position.z});

      glm_translate(mat_model,
                    (vec3){node.transform.position.x, node.transform.position.y,
                           node.transform.position.z});

      glm_scale(mat_model,
                      (vec3){node.transform.scale.x, node.transform.scale.y,
                            node.transform.scale.z});

      glUniformMatrix4fv(glGetUniformLocation(program, "u_model"), 1,
                         GL_FALSE, (float *)mat_model);

      draw_mesh(&node.mesh, program);
    }
}

unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
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
  glfwSetCursorPosCallback(window, cursor_pos_callback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  printf("Initializing GLAD...\n");

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    printf("Failed to initialize GLAD.\n");
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  printf("GLAD initialized successsfuly.\n");

  t_scene *scenes = NULL;
  // process_gltf_file("./res/models/simple/scene.gltf", &model);
  // process_gltf_file("./res/models/triangle/Triangle.gltf", &model);
  // process_gltf_file("./res/models/triangle_without_indices/triangleWithoutIndices.gltf",
  // &model); process_gltf_file("./res/models/cube/Cube.gltf", &scenes);
  // process_gltf_file("./res/models/avocado/Avocado.gltf", &scenes);
  // process_gltf_file("./res/models/corset/Corset.gltf", &model);
  // &model);
  // process_gltf_file("./res/models/simple_meshes/SimpleMeshes.gltf", &scenes);
    process_gltf_file("./res/scenes/scene_3.gltf", &scenes);

  if (scenes == NULL) {
    printf("Could not load scenes.\n");
  } else
    printf("Scenes loaded successfuly.\n");

  t_scene scene = scenes[0];
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  glViewport(0, 0, width, height);
  const float ratio = width / (float)height;

   unsigned int depth_shader_program = create_shader_program(
     "./res/shaders/depth_shader.vs", "./res/shaders/depth_shader.fs");

  unsigned int depth_shader_debug_program = create_shader_program(
    "./res/shaders/depth_shader_debug.vs", "./res/shaders/depth_shader_debug.fs");

  unsigned int shader_program = create_shader_program(
      "./res/shaders/shader.vs", "./res/shaders/shader.fs");

  
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_STENCIL_TEST);
  glEnable(GL_CULL_FACE);


  //framebuffer
  unsigned int depth_map_framebuffer;
  glGenFramebuffers(1, &depth_map_framebuffer);

  const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

  //depth map texture
  unsigned int depth_map_texture;
  glGenTextures(1, &depth_map_texture);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, depth_map_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
              SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); 

  //bind framebuffer and add texture
  glBindFramebuffer(GL_FRAMEBUFFER, depth_map_framebuffer);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_map_texture, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  glUseProgram(shader_program);
  glUniform3fv(glGetUniformLocation(shader_program, "light_pos"), 1, light_pos);
  glUniform3fv(glGetUniformLocation(shader_program, "light_color"), 1,
               (vec3){1, 1, 1});
  glUniform1i(glGetUniformLocation(shader_program, "u_shadow_map"), 0);

  while (!glfwWindowShouldClose(window)) {

    calculate_delta_time();

    process_input(window);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glClearColor(36.0 / 255, 10.0 / 255, 52.0 / 255, 1);

    glUniform1i(glGetUniformLocation(shader_program, "use_blinn"), use_blinn);

    // identity matrix
    mat4 mat_view;
    glm_mat4_identity(mat_view);
    glm_look(cam_pos, cam_dir, cam_up, mat_view);

    mat4 mat_projection;
    glm_perspective(glm_rad(45.0f), ratio, .1f, 100.0f, mat_projection);

    //shadow map
    glUseProgram(depth_shader_program);

    mat4 mat_light_projection;
    glm_ortho(-10.0, 10.0, -10.0, 10.0, 1.0, 27.5, mat_light_projection);

    mat4 mat_light_view;
    glm_mat4_identity(mat_light_view);
    glm_look(light_pos, (vec3){-.45f, -0.65f, -0.45f}, cam_up, mat_light_view);

    mat4 mat_light_space;
    glm_mul(mat_light_projection, mat_light_view, mat_light_space);

    glCullFace(GL_FRONT);
    glUseProgram(depth_shader_program);
    glUniformMatrix4fv(glGetUniformLocation(depth_shader_program, "light_space_matrix"), 1,
                       GL_FALSE, (float *)mat_light_space);

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depth_map_framebuffer);
    glClear(GL_DEPTH_BUFFER_BIT);
    render_scene(&scene, depth_shader_program);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glCullFace(GL_BACK);
    glUseProgram(shader_program);
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glUniformMatrix4fv(glGetUniformLocation(shader_program, "u_view"), 1,
                       GL_FALSE, (float *)mat_view);

    glUniformMatrix4fv(glGetUniformLocation(shader_program, "u_projection"), 1,
                       GL_FALSE, (float *)mat_projection);

    glUniform3fv(glGetUniformLocation(shader_program, "view_pos"), 1, cam_pos);
    glUniformMatrix4fv(glGetUniformLocation(shader_program, "u_light_space_matrix"), 1,
                       GL_FALSE, (float *)mat_light_space);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depth_map_texture);   
    render_scene(&scene, shader_program);


    // glUseProgram(depth_shader_debug_program);
    // glUniform1f(glGetUniformLocation(depth_shader_debug_program, "near_plane"), 1.0f);
    // glUniform1f(glGetUniformLocation(depth_shader_debug_program, "far_plane"), 7.5f);
    // glActiveTexture(GL_TEXTURE0);
    // glUniform1i(glGetUniformLocation(depth_shader_debug_program, "depthMap"), 0);
    // renderQuad();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // glDeleteVertexArrays(1, &vertex_array_object);
  //  glDeleteBuffers(1, &VBO);
  //  glDeleteBuffers(1, &EBO);
  //glDeleteProgram(shader_program);

  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}



bool was_space_pressed;
bool was_b_pressed;

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

  if (!was_space_pressed && glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
    was_space_pressed = true;
  }

  if (was_space_pressed && glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) {
    was_space_pressed = false;
    rotate_model = !rotate_model;
  }

  if (!was_b_pressed && glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
    was_b_pressed = true;
  }

  if (was_b_pressed && glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE) {
    was_b_pressed = false;
    use_blinn = !use_blinn;
  }
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
