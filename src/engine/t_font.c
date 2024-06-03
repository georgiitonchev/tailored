#include "tailored.h"

#include <math.h>
#include <string.h>

// GL loader
#include "../../dep/include/glad/glad.h"

// MATH
#include "../../dep/include/cglm/cglm.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "../engine/extern/stb_truetype.h"

static unsigned int font_quad_vao;
static unsigned int font_shader;
static t_texture font_texture;

static mat4 s_mat4_projection;

static float s_bake_font_bitmap(stbtt_fontinfo *font_info,
                                float pixel_height,                     // height of font in pixels
                                unsigned char *pixels, int pw, int ph,  // bitmap to be filled in
                                int first_char, int num_chars,          // characters to bake
                                t_font_character *font_characters)
{
   float scale;
   int x,y,bottom_y, i;

   STBTT_memset(pixels, 0, pw*ph); // background of 0 around pixels
   x=y=1;
   bottom_y = 1;

   scale = stbtt_ScaleForPixelHeight(font_info, pixel_height);

   for (i=0; i < num_chars; ++i) {
      int advance, lsb, x0,y0,x1,y1,gw,gh;
      int g = stbtt_FindGlyphIndex(font_info, first_char + i);
      stbtt_GetGlyphHMetrics(font_info, g, &advance, &lsb);
      stbtt_GetGlyphBitmapBox(font_info, g, scale, scale, &x0, &y0, &x1, &y1);
      gw = x1-x0;
      gh = y1-y0;
      if (x + gw + 1 >= pw)
         y = bottom_y, x = 1; // advance to next row
      if (y + gh + 1 >= ph) // check if it fits vertically AFTER potentially moving to next row
         return -i;
      STBTT_assert(x+gw < pw);
      STBTT_assert(y+gh < ph);
      stbtt_MakeGlyphBitmap(font_info, pixels+x+y*pw, gw,gh,pw, scale,scale, g);
      font_characters[i].x = (stbtt_int16) x;
      font_characters[i].y = (stbtt_int16) y;
      font_characters[i].width = (stbtt_int16) (gw);
      font_characters[i].height = (stbtt_int16) (gh);
      font_characters[i].advance = scale * advance;
      font_characters[i].bearing_x = scale * lsb;
      font_characters[i].xoff     = (float) x0;
      font_characters[i].yoff     = (float) y0;
      x = x + gw + 1;
      if (y+gh+1 > bottom_y)
         bottom_y = y+gh+1;
   }

   return scale;
}


static void s_init_quad() {
  // configure VAO/VBO
  unsigned int vertex_buffer_object;

  float vertices[] = {
      // pos      // tex
      0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,

      0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f};

  glGenVertexArrays(1, &font_quad_vao);
  glGenBuffers(1, &vertex_buffer_object);

  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glBindVertexArray(font_quad_vao);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

static void s_init_shader() {
  font_shader = t_create_shader_program("./res/shaders/font_shader.vs",
                                        "./res/shaders/font_shader.fs");
}

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

static t_vec4 s_get_character(char character) {
  int columns = 16;

  int row = ((int)character) / columns;
  int column = ((int)character % columns);

  int left = column * 32;
  int top = row * 32;

  t_vec4 slice = {left, top, 32, 32};
  return slice;
}

t_font t_load_ttf_font(const char* path, unsigned int font_size) {

  t_font font = { 0 };
  font.characters = malloc(96 * sizeof(t_font_character));

  long file_size;
  unsigned char* file_data = t_read_file(path, &file_size);

  unsigned char bitmap_data[512 * 512];

  stbtt_fontinfo font_info = {0};
  font_info.userdata = NULL;

  if (!stbtt_InitFont(&font_info, file_data, 0))
    printf("Eror initializing font. \n");

  float scale = s_bake_font_bitmap(&font_info, font_size, bitmap_data, 512, 512, 32, 96, font.characters);

  int font_ascent = 0;
  int font_descent = 0;
  int line_gap = 0;

  stbtt_GetFontVMetrics(&font_info, &font_ascent, &font_descent, &line_gap);

  t_texture_data texture_data;
  texture_data.channels = 1;
  texture_data.bytes = bitmap_data;
  texture_data.width = 512;
  texture_data.height = 512;

  font.bitmap = s_load_texture(&texture_data);
  font.line_height = (font_ascent - font_descent + line_gap) * scale; //ascent - *descent + *lineGap

  free((void*) file_data);
  return font;
}

void t_delete_ttf_font(t_font* font) {
  free(font->characters);
  t_free_texture(&font->bitmap);
}

void t_init_font_renderer() {
    s_init_quad();
    s_init_shader();

    glm_ortho(0, 640, 360, 0, 0.0, 1.0, s_mat4_projection);

    font_texture = t_load_texture("./res/textures/font.png");
}

void t_terminate_font_renderer() {
    t_free_texture(&font_texture);
}

t_vec2 t_measure_text_size(const char* text, t_font* font) {

  t_vec2 size = VEC2_ZERO;

  size_t string_length = strlen(text);

  for (int i = 0; i < (int) string_length; i++) {
    int character = text[i] - 32;

    size.x += font->characters[character].advance;
    size.y += font->characters[character].height;
  }

  size.y /= string_length;
  return size;
}

t_vec2 t_measure_text_size_w(const char* text, t_font* font, int max_width) {

  t_vec2 size = VEC2_ZERO;

  int text_length = (int) strlen(text);

  int current_width = 0;
  int height = font->line_height * 2;
  int width = 0;

  int current_word_width = 0;
  int current_word_index = 0;
  int current_word_length = 0;

  int index = 0;

  for (int i = 0; i < text_length; i++) {

    int character = text[i] - 32;

    if (character != 0) {

      current_word_length ++;
      current_word_width += font->characters[character].advance;
    }

    if (character == 0 || i == text_length - 1) {

      if (current_word_length > 0) {

        if (max_width != 0 && current_width + current_word_width >= max_width) {
          
          if (current_width > width) 
            width = current_width;
          
          current_width = 0;
          height += font->line_height;
        }

        for (int j = current_word_index; j < current_word_index + current_word_length; j++) {

          int w_character = text[j] - 32;
          current_width += font->characters[w_character].advance;
        }

        current_word_index = i + 1;
        current_word_width = 0;
        current_word_length = 0;
      }
    }
  }

  size.x = width;
  size.y = height;

  return size;
}

void t_draw_text(const char* text, t_font* font, int x, int y, t_color color, int max_width) {

  glUseProgram(font_shader);

  glUniformMatrix4fv(glGetUniformLocation(font_shader, "u_mat4_projection"),
                      1, GL_FALSE, (float *)s_mat4_projection);

  glUniform4fv(glGetUniformLocation(font_shader, "u_color"), 1,
                      (vec4){color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f});

  glBindVertexArray(font_quad_vao);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, font->bitmap.id);

  int text_length = (int) strlen(text);
  int pos_x = x;
  int pos_y = y;

  int current_width = 0;

  int current_word_width = 0;
  int current_word_index = 0;
  int current_word_length = 0;

  int index = 0;

  for (int i = 0; i < text_length; i++) {

    int character = text[i] - 32;

    if (character != 0) {

      current_word_length ++;
      current_word_width += font->characters[character].advance;
    }

    if (character == 0 || i == text_length - 1) {

      if (current_word_length > 0) {

        if (max_width != 0 && current_width + current_word_width >= max_width) {

          current_width = 0;
          pos_x = x;
          pos_y += font->line_height;

          if (t_clip_area().height > 0 && pos_y - font->line_height > t_clip_area().y + t_clip_area().height) {
            glDrawArraysInstanced(GL_TRIANGLES, 0, 6, index);
            glBindVertexArray(0);
            glUseProgram(0);
            return;
          }
        }

        for (int j = current_word_index; j < current_word_index + current_word_length; j++) {

          int w_character = text[j] - 32;
          t_vec4 texture_slice;
          texture_slice.x = font->characters[w_character].x;
          texture_slice.y = font->characters[w_character].y;
          texture_slice.z = font->characters[w_character].width;
          texture_slice.w = font->characters[w_character].height;

          t_vec2 position = (t_vec2){ pos_x + font->characters[w_character].bearing_x, pos_y + font->characters[w_character].yoff };
          t_vec2 size = (t_vec2){ texture_slice.z, texture_slice.w };

          mat4 mat4_model;
          glm_mat4_identity(mat4_model);
          glm_translate(mat4_model,
                        (vec3){position.x, position.y, 0});

          glm_scale(mat4_model, (vec3){size.x, size.y, 1.0f});

          char model_key[30];
          sprintf(model_key, "u_mat4_models[%d]", index);
          glUniformMatrix4fv(glGetUniformLocation(font_shader, model_key), 1,
                              GL_FALSE, (float *)mat4_model);

          char slice_key[30];
          sprintf(slice_key, "u_texture_slices[%d]", index);
          glUniform4fv(glGetUniformLocation(font_shader, slice_key), 1,
                        (vec4){texture_slice.x, texture_slice.y,
                                texture_slice.z, texture_slice.w});

          pos_x += font->characters[w_character].advance;
          current_width += font->characters[w_character].advance;

          index ++;

          if (index == 128) {
            glDrawArraysInstanced(GL_TRIANGLES, 0, 6, index);
            index = 0;
          }
        }

        current_word_index = i + 1;
        current_word_width = 0;
        current_word_length = 0;
      }

      if (character == 0) {
        pos_x += font->characters[0].advance;
      }
    }
  }

  glDrawArraysInstanced(GL_TRIANGLES, 0, 6, index);

  glBindVertexArray(0);
  glUseProgram(0);
}
