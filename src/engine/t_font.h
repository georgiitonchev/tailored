#pragma once
#include "tailored.h"

typedef struct t_glyph_info { 
    int value;
    int offset_x;
    int offset_y;
    int advance_x;
    t_texture_data texture_data; 
} t_glyph_info;

typedef struct t_font_character {

    unsigned int x;
    unsigned int y;
    unsigned int width;
    unsigned int height;

    unsigned int advance;
    unsigned int bearing_x;

    float xoff;
    float yoff;

} t_font_character;

typedef struct t_font {

    t_font_character* characters;
    t_texture bitmap;
    int line_height;
    
} t_font;

void init_font_renderer();
void terminate_font_renderer();

t_font load_ttf_font(const char* path, unsigned int font_size);
void delete_ttf_font(t_font* font);

t_vec4 get_character(char character);
void draw_text(const char* text, t_vec2 position, int size, t_color color);
void draw_text_ttf(const char* text, t_font* font, t_vec2 position, t_color color, int max_width);

t_vec2 measure_text_size(const char* text, int size);
t_vec2 measure_text_size_ttf(const char* text, t_font* fonts);

t_glyph_info* t_load_font_glyphs(const unsigned char *fileData, int dataSize, int fontSize);
t_font_character* t_get_font_characters(t_glyph_info** glyphs, unsigned int glyphs_count);

t_texture_data t_pack_font_glyphs(const t_glyph_info *glyphs, t_rect **glyphRecs, int glyphCount, int fontSize);