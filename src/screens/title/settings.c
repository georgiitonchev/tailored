#include "screen_title.h"

#include "../../engine/tailored.h"
#include "../../engine/t_input.h"
#include "../../engine/t_sprite.h"
#include "../../engine/t_ui.h"
#include "../../engine/t_shapes.h"
#include "../../engine/t_font.h"

#include "../../../dep/include/glad/glad.h"

#include "../../game.h"

#include <stdio.h>

// FONTS
static t_font s_font_ui_l;
static t_font s_font_ui_s;

// SPRITES
static t_sprite s_sprite_section_background;
static t_sprite s_sprite_slider_background;
static t_sprite s_sprite_small_knob;
static t_sprite s_sprite_big_knob;

// BUTTONS
static t_ui_button s_button_slider_master_volume_knob;
static t_ui_button s_button_slider_music_volume_knob;
static t_ui_button s_button_slider_effects_volume_knob;

// STATE
static int s_volume_master = 100;
static int s_volume_music = 100;
static int s_volume_effects = 100;

static t_rect s_rect_master_volume_slider;
static t_rect s_rect_music_volume_slider;
static t_rect s_rect_effects_volume_slider;

static void s_on_master_volume_slider_knob_button_pressed() { 
    float value = (mouse_position().x - s_rect_master_volume_slider.x) / s_rect_master_volume_slider.width;

    if (value < 0)
        value = 0;
    else if (value > 1)
        value = 1;

    t_set_master_volume(value);
    s_volume_master = 100 * value;
}

static void s_on_music_volume_slider_knob_button_pressed() { 
    float value = (mouse_position().x - s_rect_music_volume_slider.x) / s_rect_music_volume_slider.width;

    if (value < 0)
        value = 0;
    else if (value > 1)
        value = 1;
    
    s_volume_music = 100 * value;
}

static void s_on_effects_volume_slider_knob_button_pressed() { 
    float value = (mouse_position().x - s_rect_effects_volume_slider.x) / s_rect_effects_volume_slider.width;

    if (value < 0)
        value = 0;
    else if (value > 1)
        value = 1;
    
    s_volume_effects = 100 * value;
}

void load_section_settings() {

    s_font_ui_s = load_ttf_font("./res/fonts/Eczar-Regular.ttf", 32);
    s_font_ui_l = load_ttf_font("./res/fonts/Eczar-Regular.ttf", 42);

    create_sprite("./res/textures/panel-transparent-center-030.png", &s_sprite_section_background);
    s_sprite_section_background.slice_borders = (t_vec4){ 16, 16, 16, 16 };

    create_sprite("./res/textures/slider_background_slim.png", &s_sprite_slider_background);
    create_sprite("./res/textures/slider_knob_small_slim.png", &s_sprite_small_knob);
    create_sprite("./res/textures/slider_knob_big_slim.png", &s_sprite_big_knob);

    s_button_slider_master_volume_knob = create_ui_button(&s_sprite_big_knob);
    s_button_slider_master_volume_knob.color_default = CC_BLACK;
    s_button_slider_master_volume_knob.color_mouseover = CC_DARK_RED;
    s_button_slider_master_volume_knob.color_clicked = CC_BLACK;
    s_button_slider_master_volume_knob.on_pressed = s_on_master_volume_slider_knob_button_pressed;

    s_button_slider_music_volume_knob = create_ui_button(&s_sprite_big_knob);
    s_button_slider_music_volume_knob.color_default = CC_BLACK;
    s_button_slider_music_volume_knob.color_mouseover = CC_DARK_RED;
    s_button_slider_music_volume_knob.color_clicked = CC_BLACK;
    s_button_slider_music_volume_knob.on_pressed = s_on_music_volume_slider_knob_button_pressed;

    s_button_slider_effects_volume_knob = create_ui_button(&s_sprite_big_knob);
    s_button_slider_effects_volume_knob.color_default = CC_BLACK;
    s_button_slider_effects_volume_knob.color_mouseover = CC_DARK_RED;
    s_button_slider_effects_volume_knob.color_clicked = CC_BLACK;
    s_button_slider_effects_volume_knob.on_pressed = s_on_effects_volume_slider_knob_button_pressed;
}

void draw_section_settings(const float p_offset_x, const float p_offset_y) {

    draw_sprite(&s_sprite_section_background, 256, 16 + p_offset_y, 368, 328, CC_LIGHT_RED);

    t_vec2 text_size_sound = measure_text_size_ttf("Sound", &s_font_ui_l);
    draw_text_ttf("Sound", &s_font_ui_l, (t_vec2) {256 + (368 - text_size_sound.x) / 2, 48 + p_offset_y}, CC_BLACK, 0);

    // MASTER VOLUME
    draw_text_ttf("Master", &s_font_ui_s, (t_vec2) { 272, 80 + p_offset_y}, CC_BLACK, 0);

    s_rect_master_volume_slider = (t_rect) { 340, 70 + p_offset_y, 200, s_sprite_slider_background.texture.size.y };
    
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glClear(GL_STENCIL_BUFFER_BIT);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);
    draw_ui_button(&s_button_slider_master_volume_knob, 340 + (((float)s_volume_master / 100) * 200) - s_sprite_big_knob.texture.size.x / 2, s_rect_master_volume_slider.y + (s_sprite_slider_background.texture.size.y - s_sprite_big_knob.texture.size.y) / 2, s_sprite_big_knob.texture.size.x, s_sprite_big_knob.texture.size.y);

    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    if (!s_button_slider_master_volume_knob.is_mouse_over && is_point_in_rect(mouse_position(), s_rect_master_volume_slider)) {

        if (is_mouse_button_pressed(MOUSE_BUTTON_LEFT)) {

            float value = (mouse_position().x - s_rect_master_volume_slider.x) / s_rect_master_volume_slider.width;
            s_volume_master = 100 * value;

            s_button_slider_master_volume_knob.was_clicked = true;
            s_button_slider_master_volume_knob.mouse_clicked_at = (t_vec2) { s_button_slider_master_volume_knob.sprite->texture.size.x / 2, s_button_slider_master_volume_knob.sprite->texture.size.y / 2 };
        }

        t_rect slider_knob_small_rect = (t_rect) 
            { mouse_position().x - s_sprite_small_knob.texture.size.x / 2, 
            s_rect_master_volume_slider.y + p_offset_y, 
            s_sprite_small_knob.texture.size.x, 
            s_sprite_small_knob.texture.size.y };

        draw_sprite_t(&s_sprite_small_knob, slider_knob_small_rect, CC_BLACK);
    }

    glStencilMask(0x00);
    
    draw_sprite_t(&s_sprite_slider_background, s_rect_master_volume_slider, CC_BLACK);
    glStencilMask(0xFF);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);

    char value_str[4];
    sprintf(value_str, "%d", s_volume_master);
    draw_text_ttf(value_str, &s_font_ui_s, (t_vec2) { 368 + 176 + 16, 80 + p_offset_y}, CC_BLACK, 0);

    // MUSIC VOLUME
    draw_text_ttf("Music", &s_font_ui_s, (t_vec2) {256 + 16, 112 + p_offset_y}, CC_BLACK, 0);

    s_rect_music_volume_slider = (t_rect) {  256 + 184 - 100, 102 + p_offset_y, 200, s_sprite_slider_background.texture.size.y };
    
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glClear(GL_STENCIL_BUFFER_BIT);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);
    draw_ui_button(&s_button_slider_music_volume_knob, 340 + (((float)s_volume_music / 100) * 200) - s_sprite_big_knob.texture.size.x / 2, s_rect_music_volume_slider.y + (s_sprite_slider_background.texture.size.y - s_sprite_big_knob.texture.size.y) / 2, s_sprite_big_knob.texture.size.x, s_sprite_big_knob.texture.size.y);

    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    if (!s_button_slider_music_volume_knob.is_mouse_over && is_point_in_rect(mouse_position(), s_rect_music_volume_slider)) {

        if (is_mouse_button_pressed(MOUSE_BUTTON_LEFT)) {

            float value = (mouse_position().x - s_rect_music_volume_slider.x) / s_rect_music_volume_slider.width;
            s_volume_music = 100 * value;

            s_button_slider_music_volume_knob.was_clicked = true;
            s_button_slider_music_volume_knob.mouse_clicked_at = (t_vec2) { s_button_slider_music_volume_knob.sprite->texture.size.x / 2, s_button_slider_music_volume_knob.sprite->texture.size.y / 2 };
        }

        t_rect slider_knob_small_rect = (t_rect) 
            { mouse_position().x - s_sprite_small_knob.texture.size.x / 2, 
            s_rect_music_volume_slider.y + p_offset_y, 
            s_sprite_small_knob.texture.size.x, 
            s_sprite_small_knob.texture.size.y };

        draw_sprite_t(&s_sprite_small_knob, slider_knob_small_rect, CC_BLACK);
    }

    glStencilMask(0x00);
    draw_sprite_t(&s_sprite_slider_background, s_rect_music_volume_slider, CC_BLACK);
    glStencilMask(0xFF);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);

    char volume_music_str[4];
    sprintf(volume_music_str, "%d", s_volume_music);
    draw_text_ttf(volume_music_str, &s_font_ui_s, (t_vec2) { 368 + 176 + 16, 112 + p_offset_y }, CC_BLACK, 0);

    draw_text_ttf("Effects", &s_font_ui_s, (t_vec2) {256 + 16, 48 + 48 + 24 + 24 + p_offset_y}, CC_BLACK, 0);

    s_rect_effects_volume_slider = (t_rect) {  256 + 184 - 100, 134 + p_offset_y, 200, s_sprite_slider_background.texture.size.y };

    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glClear(GL_STENCIL_BUFFER_BIT);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);
    draw_ui_button(&s_button_slider_effects_volume_knob, 340 + (((float)s_volume_effects / 100) * 200) - s_sprite_big_knob.texture.size.x / 2, s_rect_effects_volume_slider.y + (s_sprite_slider_background.texture.size.y - s_sprite_big_knob.texture.size.y) / 2, s_sprite_big_knob.texture.size.x, s_sprite_big_knob.texture.size.y);

    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    if (!s_button_slider_effects_volume_knob.is_mouse_over && is_point_in_rect(mouse_position(), s_rect_effects_volume_slider)) {

        if (is_mouse_button_pressed(MOUSE_BUTTON_LEFT)) {

            float value = (mouse_position().x - s_rect_effects_volume_slider.x) / s_rect_effects_volume_slider.width;
            s_volume_effects = 100 * value;

            s_button_slider_effects_volume_knob.was_clicked = true;
            s_button_slider_effects_volume_knob.mouse_clicked_at = (t_vec2) { s_button_slider_effects_volume_knob.sprite->texture.size.x / 2, s_button_slider_effects_volume_knob.sprite->texture.size.y / 2 };
        }

        t_rect slider_knob_small_rect = (t_rect) 
            { mouse_position().x - s_sprite_small_knob.texture.size.x / 2, 
            s_rect_effects_volume_slider.y + p_offset_y, 
            s_sprite_small_knob.texture.size.x, 
            s_sprite_small_knob.texture.size.y };

        draw_sprite_t(&s_sprite_small_knob, slider_knob_small_rect, CC_BLACK);
    }

    glStencilMask(0x00);
    draw_sprite_t(&s_sprite_slider_background, s_rect_effects_volume_slider, CC_BLACK);
    glStencilMask(0xFF);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);

    char volume_effects_str[4];
    sprintf(volume_effects_str, "%d", s_volume_effects);
    draw_text_ttf(volume_effects_str, &s_font_ui_s, (t_vec2) { 368 + 176 + 16, 48 + 48 + 24 + 24 + p_offset_y}, CC_BLACK, 0);
}

void unload_section_settings() {
    
    delete_ttf_font(&s_font_ui_s);
    delete_ttf_font(&s_font_ui_l);

    delete_sprite(&s_sprite_section_background);
    delete_sprite(&s_sprite_slider_background);
    delete_sprite(&s_sprite_small_knob);
    delete_sprite(&s_sprite_big_knob);
}