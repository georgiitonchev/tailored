#include "screens.h"
#include "./engine/t_sprite.h"
#include "./engine/t_ui.h"

#include "stdio.h"
#include "stdbool.h"
#include "stdlib.h"

#include "./engine/tailored.h"
#include "./engine/t_list.h"
#include "./engine/t_shapes.h"
#include "./engine/t_input.h"
#include "./engine/t_font.h"

#include "./engine/t_easings.h"

#include "../dep/include/glad/glad.h"

#include "cJSON.h"

#include "fire_particles.h"

#include "./screens/title/screen_title.h"

#define CC_LIGHT_RED (t_color) { 242, 97, 63, 255 }
#define CC_RED (t_color) { 155, 57, 34, 255 }
#define CC_DARK_RED (t_color) { 72, 30, 20, 255 }
#define CC_BLACK (t_color) { 12, 12, 12, 255 }
#define CC_LIGH_BLUE (t_color) { 109, 197, 209, 255 }

// EXTERN
extern const int SCREEN_WIDTH_DEFAULT;
extern const int SCREEN_HEIGHT_DEFAULT;
extern t_input_state input_state;

extern bool m_should_change_screen;
extern t_screen m_should_change_screen_to;

// SPRITES
static t_sprite m_logo_sprite;
static t_sprite m_button_sprite;
static t_sprite m_button_sprite_selected;

static t_sprite s_sprite_loading_bar;

// UI
//  BUTTONS
static t_ui_button m_characters_button;
static t_ui_button m_settings_button;
static t_ui_button m_about_button;

static t_font s_ui_font_l;
static t_font s_ui_font_m;
static t_font s_ui_font_s;

static bool m_draw_characters;
static bool m_draw_settings;
static bool m_draw_about;

static float s_ease_out_timer_left_side = 0;
static float s_ease_out_left_side = false;

static float s_ease_out_timer_right_side = 0;
static float s_ease_out_right_side = false;

static float s_timer_loading = 0;

static float s_ease_out_timer_characters = 0;
static bool s_ease_in_characters = false;
static bool s_ease_out_characters = false;
static float s_offset_y_characters = 0;

static float s_ease_out_timer_settings = 0;
static bool s_ease_in_settings = false;
static bool s_ease_out_settings = false;
static float s_offset_y_settings = 0;

static float s_ease_out_timer_about = 0;
static bool s_ease_in_about = false;
static bool s_ease_out_about = false;
static float s_offset_y_about = 0;

static float s_left_side_offset_x = 0;
static float s_right_side_offset_x = 0;

// BUTTON CALLBACKS

static void on_button_mouse_enter() {
     //t_play_audio("./res/audio/click_003.wav");
}

static bool s_show_loading_bar = false;

static void on_button_start_cicked(t_ui_button* button) { 
    //set_updating(false);
    s_ease_out_left_side = true;
    s_ease_out_right_side = true;
    s_show_loading_bar = true;
}

static void on_characters_button_clicked() {

    if (s_ease_in_characters || s_ease_in_about || s_ease_in_settings) return;

    if (m_draw_characters) {
        s_ease_out_characters = true;
        m_characters_button.sprite = &m_button_sprite;
        return;
    }

    m_characters_button.sprite = &m_button_sprite_selected;
    // m_characters_button.is_disabled = true;

    m_settings_button.sprite = &m_button_sprite;
    // m_settings_button.is_disabled = false;

    m_about_button.sprite = &m_button_sprite;
    // m_about_button.is_disabled = false;

    if (m_draw_settings) {
        s_ease_out_settings = true;
    }
    else if (m_draw_about) {
        s_ease_out_about = true;
    }

    s_ease_in_characters = true;
    m_draw_characters = true;
}

static void on_settings_button_clicked() {

    if (s_ease_in_settings || s_ease_in_characters || s_ease_in_about) return;

    if (m_draw_settings) {
        s_ease_out_settings = true;
        m_settings_button.sprite = &m_button_sprite;
        return;
    }

    m_characters_button.sprite = &m_button_sprite;
    // m_characters_button.is_disabled = false;

    m_settings_button.sprite = &m_button_sprite_selected;
    // m_settings_button.is_disabled = true;

    m_about_button.sprite = &m_button_sprite;
    // m_about_button.is_disabled = false;

    if (m_draw_characters) {
        s_ease_out_characters = true;
    }
    else if (m_draw_about) {
        s_ease_out_about = true;
    }

    s_ease_in_settings = true;
    m_draw_settings = true;
}

static void on_about_button_clicked() {

    if (s_ease_in_about || s_ease_in_characters || s_ease_in_settings) return;

    if (m_draw_about) {
        s_ease_out_about = true;
        m_about_button.sprite = &m_button_sprite;
        return;
    }

    m_characters_button.sprite = &m_button_sprite;
    // m_characters_button.is_disabled = false;

    m_settings_button.sprite = &m_button_sprite;
    // m_settings_button.is_disabled = false;

    m_about_button.sprite = &m_button_sprite_selected;
    // m_about_button.is_disabled = true;

    if (m_draw_characters) {
        s_ease_out_characters = true;
    }
    else if (m_draw_settings) {
        s_ease_out_settings = true;
    }

    s_ease_in_about = true;
    m_draw_about = true;
}

void load_title_screen() {

    load_section_saves();
    load_section_settings();
    load_section_about();

    s_ui_font_l = load_ttf_font("./res/fonts/Eczar-Regular.ttf", 42);
    s_ui_font_m = load_ttf_font("./res/fonts/Eczar-Regular.ttf", 37);
    s_ui_font_s = load_ttf_font("./res/fonts/Eczar-Regular.ttf", 32);

    create_sprite("./res/textures/loading_bar.png", &s_sprite_loading_bar);
    create_sprite("./res/textures/imps_fairies_logo.png", &m_logo_sprite);

    create_sprite("./res/textures/panel-transparent-center-030.png", &m_button_sprite);
    m_button_sprite.slice_borders = (t_vec4){ 16, 16, 16, 16 };

    create_sprite("./res/textures/panel-transparent-center-029.png", &m_button_sprite_selected);
    m_button_sprite_selected.slice_borders = (t_vec4){ 16, 16, 16, 16 };

    m_characters_button = create_ui_button(&m_button_sprite);
    m_characters_button.color_default = CC_LIGHT_RED;
    m_characters_button.color_disabled = CC_RED;
    m_characters_button.on_released = on_characters_button_clicked;
    m_characters_button.on_mouse_enter = on_button_mouse_enter;

    m_settings_button = create_ui_button(&m_button_sprite);
    m_settings_button.color_default = CC_LIGHT_RED;
    m_settings_button.color_disabled = CC_RED;
    m_settings_button.on_released = on_settings_button_clicked;
    m_settings_button.on_mouse_enter = on_button_mouse_enter;

    m_about_button = create_ui_button(&m_button_sprite);
    m_about_button.color_default = CC_LIGHT_RED;
    m_about_button.color_disabled = CC_RED;
    m_about_button.on_released = on_about_button_clicked;
    m_about_button.on_mouse_enter = on_button_mouse_enter;

    init_fire_particles(50);
}

void unload_title_screen() {

}

void update_title_screen() {

}

void draw_title_screen() {

    t_clear_color(CC_BLACK);

    draw_sprite(&m_logo_sprite, 0 + s_left_side_offset_x, 0, m_logo_sprite.texture.size.x, m_logo_sprite.texture.size.y, CC_LIGHT_RED);

    draw_fire_particles();

    // LEFT SIDE BUTTONS
    draw_ui_button(&m_characters_button, 64 + s_left_side_offset_x, 140, 128, 48);
    draw_ui_button(&m_settings_button, 64 + s_left_side_offset_x, 204, 128, 48);
    draw_ui_button(&m_about_button, 64 + s_left_side_offset_x, 268, 128, 48);

    t_vec2 text_size_play = measure_text_size_ttf("Play", &s_ui_font_l);
    draw_text_ttf("Play", &s_ui_font_l, (t_vec2) {64 + (128 - text_size_play.x) / 2 + s_left_side_offset_x , 140 + (48 + text_size_play.y) / 2}, CC_BLACK, 0);

    t_vec2 text_size_settings = measure_text_size_ttf("Settings", &s_ui_font_l);
    draw_text_ttf("Settings", &s_ui_font_l, (t_vec2) {64 + (128 - text_size_settings.x) / 2 + s_left_side_offset_x, 204 + (48 + text_size_settings.y) / 2}, CC_BLACK, 0);

    t_vec2 text_size_about = measure_text_size_ttf("About", &s_ui_font_l);
    draw_text_ttf("About", &s_ui_font_l, (t_vec2) {64 + (128 - text_size_about.x) / 2 + s_left_side_offset_x, 268 + (48 + text_size_about.y) / 2}, CC_BLACK, 0);

    if (s_ease_in_characters) {
        float progress = t_ease_out_quint(&s_ease_out_timer_characters, &s_offset_y_characters, 360, 0, .5f);

        if (progress >= 1) {
            s_ease_out_timer_characters = 0;
            s_ease_in_characters = false;
        }
    }

    if (s_ease_out_characters) {

        float progress = t_ease_out_quint(&s_ease_out_timer_characters, &s_offset_y_characters, 0, -360, .5f);

        if (progress >= 1) {
            s_ease_out_timer_characters = 0;
            s_ease_out_characters = false;
            m_draw_characters = false;
        }
    }

    if (s_ease_in_settings) {
        float progress = t_ease_out_quint(&s_ease_out_timer_settings, &s_offset_y_settings, 360, 0, .5f);

        if (progress >= 1) {
            s_ease_out_timer_settings = 0;
            s_ease_in_settings = false;
        }
    }

    if (s_ease_out_settings) {

        float progress = t_ease_out_quint(&s_ease_out_timer_settings, &s_offset_y_settings, 0, -360, .5f);

        if (progress >= 1) {
            s_ease_out_timer_settings = 0;
            s_ease_out_settings = false;
            m_draw_settings = false;
        }
    }

    if (s_ease_in_about) {
        float progress = t_ease_out_quint(&s_ease_out_timer_about, &s_offset_y_about, 360, 0, .5f);

        if (progress >= 1) {
            s_ease_out_timer_about = 0;
            s_ease_in_about = false;
        }
    }

    if (s_ease_out_about) {

        float progress = t_ease_out_quint(&s_ease_out_timer_about, &s_offset_y_about, 0, -360, .5f);

        if (progress >= 1) {
            s_ease_out_timer_about = 0;
            s_ease_out_about = false;
            m_draw_about = false;
        }
    }

    if (s_ease_out_left_side) {

        float progress = t_ease_out_quint(&s_ease_out_timer_left_side, &s_left_side_offset_x, 0, -256, .5f);

        if (progress >= 1) {
            s_ease_out_timer_left_side = 0;
            s_ease_out_left_side = false;
        }
    }

    if (s_ease_out_right_side) {

        float progress = t_ease_out_quint(&s_ease_out_timer_right_side, &s_right_side_offset_x, 0, 384, .5f);

        if (progress >= 1) {
            s_ease_out_timer_right_side = 0;
            s_ease_out_right_side = false;
        }
    }

    if (m_draw_characters) {
        draw_section_saves(s_right_side_offset_x, s_offset_y_characters);
    }
    if (m_draw_settings) {
        draw_section_settings(s_right_side_offset_x, s_offset_y_settings);
    }
    if (m_draw_about) {
        draw_section_about(s_right_side_offset_x, s_offset_y_about);
    }

    if (s_show_loading_bar) {

        draw_sprite(&s_sprite_loading_bar, 
            (t_window_size().x - s_sprite_loading_bar.texture.size.x) / 2, 
            (t_window_size().y - s_sprite_loading_bar.texture.size.y) / 2,
            s_sprite_loading_bar.texture.size.x,
            s_sprite_loading_bar.texture.size.y, CC_RED);
        
        float loading_bar_width = 0;
        float progress = t_ease_out_quint(&s_timer_loading, &loading_bar_width, 0, s_sprite_loading_bar.texture.size.x, 3.0f);

        draw_rect(
            (t_window_size().x - s_sprite_loading_bar.texture.size.x) / 2, 
            t_window_size().y / 2 - s_sprite_loading_bar.texture.size.y / 4,
            loading_bar_width,
            s_sprite_loading_bar.texture.size.y / 2, CC_RED);

        if (progress > 1) {
            s_show_loading_bar = false;
            set_updating(false);
        }
    }

    clear_ui();

}
