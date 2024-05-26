#include "screens.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "./engine/tailored.h"
#include "../dep/include/glad/glad.h"

#include "cJSON.h"

#include "fire_particles.h"

#include "./screens/title/screen_title.h"
#include "game.h"

// EXTERN
extern const int SCREEN_WIDTH_DEFAULT;
extern const int SCREEN_HEIGHT_DEFAULT;
extern bool m_should_change_screen;
extern t_screen m_should_change_screen_to;

// SPRITES
static t_sprite s_sprite_logo;
static t_sprite s_sprite_button;
static t_sprite s_sprite_button_selected;
static t_sprite s_sprite_loading_bar;

//  BUTTONS
static t_ui_button m_characters_button;
static t_ui_button m_settings_button;
static t_ui_button m_about_button;

static t_font s_ui_font_l;

static bool m_draw_characters;
static bool m_draw_settings;
static bool m_draw_about;

static float s_ease_out_timer_left_side = 0;
static float s_ease_out_left_side = false;

static float s_ease_in_left_side = false;

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

static bool s_show_loading_bar = false;

static void on_button_mouse_enter() {
     //t_play_audio("./res/audio/click_003.wav");
}

static void on_button_start_cicked(t_ui_button* button) {
    UNUSED(button);

    set_updating(false);
    s_ease_out_left_side = true;
    s_ease_out_right_side = true;
    s_show_loading_bar = true;
}

static void on_characters_button_clicked() {

    if (s_ease_in_characters || s_ease_in_about || s_ease_in_settings) return;

    if (m_draw_characters) {
        s_ease_out_characters = true;
        m_characters_button.sprite = &s_sprite_button;
        return;
    }

    m_characters_button.sprite = &s_sprite_button_selected;
    m_settings_button.sprite = &s_sprite_button;
    m_about_button.sprite = &s_sprite_button;

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
        m_settings_button.sprite = &s_sprite_button;
        return;
    }

    m_characters_button.sprite = &s_sprite_button;
    m_settings_button.sprite = &s_sprite_button_selected;
    m_about_button.sprite = &s_sprite_button;

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
        m_about_button.sprite = &s_sprite_button;
        return;
    }

    m_characters_button.sprite = &s_sprite_button;
    m_settings_button.sprite = &s_sprite_button;
    m_about_button.sprite = &s_sprite_button_selected;

    if (m_draw_characters) {
        s_ease_out_characters = true;
    }
    else if (m_draw_settings) {
        s_ease_out_settings = true;
    }

    s_ease_in_about = true;
    m_draw_about = true;
}

int load_title_screen(void* args) {
    UNUSED(args);

    s_ease_in_left_side = true;

    load_section_saves();
    load_section_settings();
    load_section_about();

    t_load_texture_data_s(&s_sprite_loading_bar, "./res/textures/loading_bar.png");
    t_load_texture_data_s(&s_sprite_logo, "./res/textures/imps_fairies_logo.png");
    t_load_texture_data_s(&s_sprite_button, "./res/textures/panel-transparent-center-030.png");
    t_load_texture_data_s(&s_sprite_button_selected, "./res/textures/panel-transparent-center-029.png");

    // RESET
    s_ease_out_timer_left_side = 0;
    s_ease_out_left_side = false;

    s_ease_out_timer_right_side = 0;
    s_ease_out_right_side = false;

    s_left_side_offset_x = 0;
    s_right_side_offset_x = 0;

    m_draw_characters = false;
    m_draw_about = false;
    m_draw_settings = false;

    s_timer_loading = 0;

    set_loading_progress(1);
    set_loading_finished();
    return 0;
}

void init_title_screen() {

    s_ui_font_l = load_ttf_font("./res/fonts/Eczar-Regular.ttf", 42);

    t_init_sprite(&s_sprite_loading_bar);
    t_init_sprite(&s_sprite_logo);
    t_init_sprite(&s_sprite_button);
    s_sprite_button.slice_borders = (t_vec4){ 16, 16, 16, 16 };

    t_init_sprite(&s_sprite_button_selected);
    s_sprite_button_selected.slice_borders = (t_vec4){ 16, 16, 16, 16 };

    m_characters_button = create_ui_button(&s_sprite_button);
    m_characters_button.color_default = CC_LIGHT_RED;
    m_characters_button.color_disabled = CC_RED;
    m_characters_button.on_released = on_characters_button_clicked;
    m_characters_button.on_mouse_enter = on_button_mouse_enter;

    m_settings_button = create_ui_button(&s_sprite_button);
    m_settings_button.color_default = CC_LIGHT_RED;
    m_settings_button.color_disabled = CC_RED;
    m_settings_button.on_released = on_settings_button_clicked;
    m_settings_button.on_mouse_enter = on_button_mouse_enter;

    m_about_button = create_ui_button(&s_sprite_button);
    m_about_button.color_default = CC_LIGHT_RED;
    m_about_button.color_disabled = CC_RED;
    m_about_button.on_released = on_about_button_clicked;
    m_about_button.on_mouse_enter = on_button_mouse_enter;

    init_section_saves();
    set_on_save_file_loaded(on_button_start_cicked);

    init_section_settings();
    init_section_about();

    init_fire_particles(50);
}

void unload_title_screen() {
    uninit_fire_particles();

    unload_section_saves();
    unload_section_settings();
    unload_section_about();

    t_deinit_sprite(&s_sprite_logo);
    t_deinit_sprite(&s_sprite_button);
    t_deinit_sprite(&s_sprite_button_selected);
    t_deinit_sprite(&s_sprite_loading_bar);

    delete_ttf_font(&s_ui_font_l);
}

void draw_title_screen() {

    t_clear_color(CC_BLACK);

    if (s_ease_in_left_side) {

        float progress = t_ease_out_quint(&s_ease_out_timer_left_side, &s_left_side_offset_x, -256, 0, .5f);

        if (progress >= 1) {
            s_ease_out_timer_left_side = 0;
            s_ease_in_left_side = false;
        }
    }

    if (s_ease_out_left_side) {

        float progress = t_ease_out_quint(&s_ease_out_timer_left_side, &s_left_side_offset_x, 0, -256, .5f);

        if (progress >= 1) {
            s_ease_out_timer_left_side = 0;
            s_ease_out_left_side = false;
        }
    }

    t_draw_sprite(&s_sprite_logo, 0 + s_left_side_offset_x, 0, s_sprite_logo.texture_data.width, s_sprite_logo.texture_data.height, CC_LIGHT_RED);

    draw_fire_particles();

    // LEFT SIDE BUTTONS
    draw_ui_button(&m_characters_button, 64 + s_left_side_offset_x, 140, 128, 48);
    draw_ui_button(&m_settings_button, 64 + s_left_side_offset_x, 204, 128, 48);
    draw_ui_button(&m_about_button, 64 + s_left_side_offset_x, 268, 128, 48);

    t_vec2 text_size_play = measure_text_size_ttf("Play", &s_ui_font_l);
    draw_text_ttf("Play", &s_ui_font_l, (t_vec2) {64 + (128 - text_size_play.x) / 2 + s_left_side_offset_x , 140 + (48 + text_size_play.y) / 2}, CC_BLACK, 0);

    t_vec2 text_size_settings = measure_text_size_ttf("Settings", &s_ui_font_l);
    draw_text_ttf("Settings", &s_ui_font_l, (t_vec2) {64 + (128 - text_size_settings.x) / 2 + s_left_side_offset_x, 204 + (48 + text_size_settings.y) / 2}, CC_BLACK, 0);

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


    if (s_ease_out_right_side) {

        float progress = t_ease_out_quint(&s_ease_out_timer_right_side, &s_right_side_offset_x, 0, 384, .5f);

        if (progress >= 1) {
            s_ease_out_timer_right_side = 0;
            s_ease_out_right_side = false;
        }
    }

    t_vec2 text_size_about = measure_text_size_ttf("About", &s_ui_font_l);
    draw_text_ttf("About", &s_ui_font_l, (t_vec2) {64 + (128 - text_size_about.x) / 2 + s_left_side_offset_x, 268 + (48 + text_size_about.y) / 2}, CC_BLACK, 0);

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
        t_draw_sprite(&s_sprite_loading_bar,
            (t_window_size().x - s_sprite_loading_bar.texture_data.width) / 2,
            (t_window_size().y - s_sprite_loading_bar.texture_data.height) / 2,
            s_sprite_loading_bar.texture_data.width,
            s_sprite_loading_bar.texture_data.height, CC_RED);

        float loading_bar_width = 0;
        float progress = t_ease_out_quint(&s_timer_loading, &loading_bar_width, 0, s_sprite_loading_bar.texture_data.width, 3.0f);

        draw_rect(
            (t_window_size().x - s_sprite_loading_bar.texture_data.width) / 2,
            t_window_size().y / 2 - s_sprite_loading_bar.texture_data.height / 4,
            loading_bar_width,
            s_sprite_loading_bar.texture_data.height / 2, CC_RED);

        if (progress > 1) {
            s_show_loading_bar = false;

            // set_screen(GAME);
            set_screen(SETUP);
        }
    }

    clear_ui();
}
