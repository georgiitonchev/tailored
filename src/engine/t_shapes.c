#include "tailored.h"

#include "../../dep/include/glad/glad.h"

static t_sprite s_white_rectangle_sprite;
static t_sprite s_white_border_rectangle_sprite; // 1 width

static t_texture s_load_texture(const t_texture_data* texture_data) {

  GLenum texture_format = 0;
  if (texture_data->channels == 1)
    texture_format = GL_RED;
  else if (texture_data->channels == 3)
    texture_format = GL_RGB;
  else if (texture_data->channels == 4)
    texture_format = GL_RGBA;

  unsigned int texture_id;
  
  glGenTextures(1, &texture_id);
  glBindTexture(GL_TEXTURE_2D, texture_id);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glTexImage2D(GL_TEXTURE_2D, 0, texture_format, texture_data->width, texture_data->height, 0,
              texture_format, GL_UNSIGNED_BYTE, texture_data->bytes);

  glBindTexture(GL_TEXTURE_2D, 0);

  t_texture texture;
  texture.id = texture_id;

  return texture;
}

GLuint s_line_vao;
GLuint s_line_vbo;

unsigned int s_line_shader_program;

static void s_init_line_renderer() { 

    float lineVertices[] = {
    -0.5f, -0.5f, // Vertex 1
     0.5f,  0.5f  // Vertex 2
    };

    glGenVertexArrays(1, &s_line_vao);
    glGenBuffers(1, &s_line_vbo);

    glBindVertexArray(s_line_vao);

    glBindBuffer(GL_ARRAY_BUFFER, s_line_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lineVertices), lineVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    s_line_shader_program = t_create_shader_program("./res/shaders/line_shader.vs", "./res/shaders/line_shader.fs");
}

void t_init_shapes_renderer() {

    unsigned char white_pixel[] = { 255, 255, 255, 255 };
    t_texture_data texture_white_data = { .bytes = (unsigned char*) white_pixel, .width = 1, .height = 1, .channels = 4 };
    t_texture texture_white = s_load_texture(&texture_white_data);
    
    s_white_rectangle_sprite.texture_data = texture_white_data;
    s_white_rectangle_sprite.texture = texture_white;
    s_white_rectangle_sprite.scale = (t_vec2){ 1, 1 };
    s_white_rectangle_sprite.texture_slice = (t_vec4){ 0, 0, s_white_rectangle_sprite.texture_data.width, s_white_rectangle_sprite.texture_data.height };
    s_white_rectangle_sprite.slice_borders = (t_vec4){ 0, 0, 0, 0 };

    unsigned char white_border_pixels[3][3][4]; // RGBA format
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            if (i == 1 && j == 1) {
                white_border_pixels[i][j][0] = 0;   // R
                white_border_pixels[i][j][1] = 0;   // G
                white_border_pixels[i][j][2] = 0;   // B
                white_border_pixels[i][j][3] = 0;   // A (Transparent)
            } else {
                white_border_pixels[i][j][0] = 255; // R
                white_border_pixels[i][j][1] = 255; // G
                white_border_pixels[i][j][2] = 255; // B
                white_border_pixels[i][j][3] = 255; // A (Opaque)
            }
        }
    }

    t_texture_data texture_white_border_data = { .bytes = (unsigned char*) white_border_pixels, .width = 3, .height = 3, .channels = 4 };
    t_texture texture_white_border = s_load_texture(&texture_white_border_data);

    s_white_border_rectangle_sprite.texture_data = texture_white_border_data;
    s_white_border_rectangle_sprite.texture = texture_white_border;
    s_white_border_rectangle_sprite.scale = (t_vec2){ 1, 1 };
    s_white_border_rectangle_sprite.texture_slice = (t_vec4){ 0, 0, s_white_border_rectangle_sprite.texture_data.width, s_white_border_rectangle_sprite.texture_data.height };
    s_white_border_rectangle_sprite.slice_borders = VEC4_ONE;

    //s_init_line_renderer();
}

bool is_point_in_rect(t_vec2 point, t_rect rect) {
    return point.x >= rect.x && point.x <= rect.x + rect.width && point.y >= rect.y && point.y <= rect.y + rect.height;
}

bool is_rect_in_rect(t_rect this, t_rect that) {
    return this.x >= that.x && this.x + this.width <= that.x + that.width &&
           this.y >= that.y && this.y + this.height <= that.y + that.height;
}

bool is_rect_zero(t_rect rect) { 
    return rect.x == 0 && rect.y == 0 && rect.width == 0 && rect.height == 0;
}

bool does_rect_overlap_rect(t_rect this, t_rect that) {

    if (this.x + this.width <= that.x || that.x + that.width <= this.x)
        return false;

    if (this.y + this.height <= that.y || that.y + that.height <= this.y)
        return false;

    return true;
}

void draw_rect(int x, int y, int width, int height, t_color color) {
    t_draw_sprite(&s_white_rectangle_sprite, x, y, width, height, color);
}

void draw_rect_lines(int x, int y, int width, int height, t_color color) {
    t_draw_sprite(&s_white_border_rectangle_sprite, x, y, width, height, color);
}

void draw_line(int x_from, int y_from, int x_to, int y_to, float width, t_color color) { 

    t_vec2 from = (t_vec2) {x_from, y_from};
    t_vec2 to = (t_vec2) {x_to, y_to};

    float distance = t_vec2_distance(from, to);
    float angle = t_vec2_angle(from, to);

    t_draw_sprite_rot(&s_white_rectangle_sprite, x_from, y_from, angle, distance, width, color);

}

// void draw_line(int x_from, int y_from, int x_to, int y_to, t_color color) { 

//     glUseProgram(s_line_shader_program);
//     glUniform4fv(glGetUniformLocation(s_line_shader_program, "u_color"), 1,
//                 (vec4){color.r / 255.0, color.g / 255.0, color.b / 255.0, color.a / 255.0});
                
//     glBindVertexArray(s_line_vao);
//     glDrawArrays(GL_LINES, 0, 2);
//     glBindVertexArray(0);
// }