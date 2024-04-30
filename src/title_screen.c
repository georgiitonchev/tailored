#include "screens.h"
#include "t_sprite.h"
#include "t_ui.h"

#include "stdio.h"
#include "stdbool.h"
#include "stdlib.h"

#include "t_list.h"
#include "t_shapes.h"
#include "t_engine.h"
#include "t_input.h"

#include "math.h"

#define CC_LIGHT_RED (t_color) { 242, 97, 63, 255 }
#define CC_RED (t_color) { 155, 57, 34, 255 }
#define CC_DARK_RED (t_color) { 72, 30, 20, 255 }
#define CC_BLACK (t_color) { 12, 12, 12, 255 }

// EXTERN
extern const int SCREEN_WIDTH_DEFAULT;
extern const int SCREEN_HEIGHT_DEFAULT;
extern t_global_state global_state;

extern bool m_should_change_screen;
extern t_screen m_should_change_screen_to;

// SPRITES
static t_sprite m_button_sprite;
static t_sprite m_button_sprite_selected;
static t_sprite m_dropdown_sprite;
static t_sprite m_slider_background_sprite;
static t_sprite m_slider_knob_small_sprite;
static t_sprite m_slider_knob_big_sprite;

// UI
//  BUTTONS
static t_ui_button m_characters_button;
static t_ui_button m_settings_button;
static t_ui_button m_about_button;

static t_ui_button m_begin_button;
static t_ui_button m_edit_button;

// SLIDER KNOB
static t_ui_button m_slider_knob_button;

//  OTHER
static t_ui_dropdown m_dropdown;
static char* dropdown_otions[] = { "Windowed", "Full-screen", "Borderless", "Full-screen Borderless" };

static bool m_draw_characters;
static bool m_draw_settings;
static bool m_draw_about;

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

static float ease_out_quint(float x) {
    return 1 - pow(1 - x, 5);
}

static float s_ease_out(float* timer, float* value, float from, float to) {

    *timer += global_state.delta_time * 5;

    *value = 
        from + 
        (to - from) * 
        ease_out_quint(*timer);

    return *timer;
}
// BUTTON CALLBACKS

static void on_button_mouse_enter() {
    // t_play_audio("./res/audio/click_003.wav");
}

static void on_characters_button_clicked() {

    if (m_draw_characters || s_ease_in_about || s_ease_in_settings) return;

    m_characters_button.sprite = &m_button_sprite_selected;
    m_settings_button.sprite = &m_button_sprite;
    m_about_button.sprite = &m_button_sprite;

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

    if (m_draw_settings || s_ease_in_characters || s_ease_in_about) return;

    m_characters_button.sprite = &m_button_sprite;
    m_settings_button.sprite = &m_button_sprite_selected;
    m_about_button.sprite = &m_button_sprite;

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

    if (m_draw_about || s_ease_in_characters || s_ease_in_settings) return;

    m_characters_button.sprite = &m_button_sprite;
    m_settings_button.sprite = &m_button_sprite;
    m_about_button.sprite = &m_button_sprite_selected;

    if (m_draw_characters) {
        s_ease_out_characters = true;
    }
    else if (m_draw_settings) {
        s_ease_out_settings = true;
    }

    s_ease_in_about = true;
    m_draw_about = true;
}

static t_ui_button* m_selected_character;

static void on_character_button_clicked(t_ui_button* button) {
    if (m_selected_character != button) {
        if (m_selected_character != NULL) { 
            m_selected_character->sprite = &m_button_sprite;
        }

        button->sprite = &m_button_sprite_selected;
        m_selected_character = button;
    }
}

static float scroll_area_width = 416;
static float scroll_area_offset = 0;

static t_vec2 slider_pos = { 272, 264 };
static float slider_width = 336;

static t_rect slider_knob_rect;
static t_list* m_characters_list;

static void on_slider_knob_button_pressed() {
    slider_knob_rect.x = global_state.mouse_pos.x - m_slider_knob_button.mouse_clicked_at.x;

    // limits
    if (slider_knob_rect.x < slider_pos.x)
        slider_knob_rect.x = slider_pos.x;
    else if (slider_knob_rect.x > slider_pos.x + slider_width - slider_knob_rect.width)
        slider_knob_rect.x = slider_pos.x + slider_width - slider_knob_rect.width;

    float normalized_knob_position = (slider_knob_rect.x - slider_pos.x) / (slider_width - slider_knob_rect.width);
    scroll_area_offset = (scroll_area_width - slider_width) * normalized_knob_position;
}

static void draw_characters() {
    // RIGHT SIDE BACKGROUND
    draw_sprite(&m_button_sprite, 256, 16 + s_offset_y_characters, 368, 328, CC_LIGHT_RED);

    // CHARACTER ELEMENTS
    // t_begin_clip_area(256 + 16 + characters_area_offset_x, 16, 368 - 32, 328);
    t_begin_scissor(256 + 16, 16 - s_offset_y_characters, 368 - 32, 328);
    for (int i = 0; i < m_characters_list->size; i ++) {

        t_ui_button* character_button = (t_ui_button*)element_at_list(m_characters_list, i);
        draw_ui_button(character_button, (272 + i * 128 + i * 16) - scroll_area_offset, 32 + s_offset_y_characters, 128, 223);
    }
    // t_end_clip_area();
    t_end_scissor();

    t_rect slider_rect = (t_rect) { 272, 328 - 64 + s_offset_y_characters, 336, m_slider_background_sprite.texture.size.y };

    t_begin_clip_area_inverse(1, slider_knob_rect.x, slider_knob_rect.y + s_offset_y_characters, slider_knob_rect.width, slider_knob_rect.height);
    if (!m_slider_knob_button.is_mouse_over && is_point_in_rect(global_state.mouse_pos, slider_rect)) {

        if (is_mouse_button_pressed(MOUSE_BUTTON_LEFT))
           slider_knob_rect.x = global_state.mouse_pos.x - slider_knob_rect.width / 2;

        t_rect slider_knob_small_rect = (t_rect) { global_state.mouse_pos.x - m_slider_knob_small_sprite.texture.size.x / 2, slider_rect.y + s_offset_y_characters, m_slider_knob_small_sprite.texture.size.x, m_slider_knob_small_sprite.texture.size.y };

        //limits
        if (slider_knob_small_rect.x < slider_pos.x)
            slider_knob_small_rect.x = slider_pos.x;
        else if (slider_knob_small_rect.x > slider_pos.x + slider_width - slider_knob_small_rect.width)
            slider_knob_small_rect.x = slider_pos.x + slider_width - slider_knob_small_rect.width;

        draw_sprite_t(&m_slider_knob_small_sprite, slider_knob_small_rect, CC_BLACK);
        t_begin_clip_area_inverse(0, slider_knob_small_rect.x, slider_rect.y + s_offset_y_characters, m_slider_knob_small_sprite.texture.size.x, m_slider_knob_small_sprite.texture.size.y);
    }

    // SLIDER
    // t_begin_clip_area_inverse_r(m_slider_knob_button.rect);
    draw_sprite_t(&m_slider_background_sprite, slider_rect, CC_BLACK);
    t_end_clip_area_inverse(0);
    t_end_clip_area_inverse(1);

    draw_ui_button(&m_slider_knob_button, slider_knob_rect.x, slider_knob_rect.y + s_offset_y_characters, slider_knob_rect.width, slider_knob_rect.height);

    // CHARACTER BUTTONS    
    draw_ui_button(&m_begin_button, 368, 292 + s_offset_y_characters, 112, 40);
    draw_ui_button(&m_edit_button, 496, 292 + s_offset_y_characters, 112, 40);
}

static void draw_settings() {
    draw_sprite(&m_button_sprite, 256, 16 + s_offset_y_settings, 368, 328, CC_LIGHT_RED);
}

static void draw_about() {
    draw_sprite(&m_button_sprite, 256, 16 + s_offset_y_about, 368, 328, CC_LIGHT_RED);
}

void load_title_screen() {

    create_sprite("./res/textures/panel-border-030.png", &m_dropdown_sprite);
    m_dropdown_sprite.slice_borders = (t_vec4){ 16, 16, 16, 16 };

    create_sprite("./res/textures/panel-transparent-center-030.png", &m_button_sprite);
    m_button_sprite.slice_borders = (t_vec4){ 16, 16, 16, 16 };

    create_sprite("./res/textures/panel-transparent-center-029.png", &m_button_sprite_selected);
    m_button_sprite_selected.slice_borders = (t_vec4){ 16, 16, 16, 16 };

    m_characters_button = create_ui_button(&m_button_sprite);
    m_characters_button.color_default = CC_LIGHT_RED;
    m_characters_button.on_released = on_characters_button_clicked;
    m_characters_button.on_mouse_enter = on_button_mouse_enter;

    m_settings_button = create_ui_button(&m_button_sprite);
    m_settings_button.color_default = CC_LIGHT_RED;
    m_settings_button.on_released = on_settings_button_clicked;
    m_settings_button.on_mouse_enter = on_button_mouse_enter;

    m_about_button = create_ui_button(&m_button_sprite);
    m_about_button.color_default = CC_LIGHT_RED;
    m_about_button.on_released = on_about_button_clicked;
    m_about_button.on_mouse_enter = on_button_mouse_enter;

    m_begin_button = create_ui_button(&m_button_sprite);
    m_begin_button.color_default = CC_BLACK;
    m_begin_button.color_mouseover = CC_DARK_RED;
    m_begin_button.color_clicked = CC_RED;
    // m_begin_button.on_released = on_quit_button_clicked;
    m_begin_button.on_mouse_enter = on_button_mouse_enter;

    m_edit_button = create_ui_button(&m_button_sprite);
    m_edit_button.color_default = CC_BLACK;
    m_edit_button.color_mouseover = CC_DARK_RED;
    m_edit_button.color_clicked = CC_RED;
    // m_edit_button.on_released = on_quit_button_clicked;
    m_edit_button.on_mouse_enter = on_button_mouse_enter;

    m_dropdown = create_ui_dropdown(&m_dropdown_sprite, (t_rect) { 128, 16 + 64 + 16, 144, 48 });

    m_dropdown.options = dropdown_otions;
    m_dropdown.options_count = 4;

    create_sprite("./res/textures/slider_background.png", &m_slider_background_sprite);
    create_sprite("./res/textures/slider_knob_small.png", &m_slider_knob_small_sprite);
    create_sprite("./res/textures/slider_knob_big.png", &m_slider_knob_big_sprite);
    m_slider_knob_button = create_ui_button(&m_slider_knob_big_sprite);
    m_slider_knob_button.color_default = CC_BLACK;
    m_slider_knob_button.color_mouseover = CC_DARK_RED;
    m_slider_knob_button.color_clicked = CC_DARK_RED;
    m_slider_knob_button.on_pressed = on_slider_knob_button_pressed;

    slider_knob_rect= (t_rect){ 272, 328 - 66, m_slider_knob_big_sprite.texture.size.x, m_slider_knob_big_sprite.texture.size.y };

    m_characters_list = create_list(sizeof(t_ui_button));
    for (int i = 0; i < 10; i++) {

        t_ui_button* character_button = (t_ui_button*)malloc(sizeof(t_ui_button));
        *character_button = create_ui_button(&m_button_sprite);

        character_button->color_default = CC_BLACK;
        character_button->color_mouseover = CC_DARK_RED;
        character_button->color_clicked = CC_RED;

        character_button->on_released = on_character_button_clicked;

        add_to_list(m_characters_list, character_button);
    }

    scroll_area_width = m_characters_list->size * 128 + (m_characters_list->size - 1) * 16;
}

void unload_title_screen() {

}

void update_title_screen() {

}

void draw_title_screen() {

    clear_color(CC_BLACK);

    // LEFT SIDE BUTTONS
    draw_ui_button(&m_characters_button, 64, 92, 128, 48);
    draw_ui_button(&m_settings_button, 64, 92 + 48 + 16, 128, 48);
    draw_ui_button(&m_about_button, 64, 92 + 48 + 48 + 16 + 16, 128, 48);

    if (s_ease_in_characters) { 
        float progress = s_ease_out(&s_ease_out_timer_characters, &s_offset_y_characters, 360, 0);

        if (progress >= 1) {
            s_ease_out_timer_characters = 0;
            s_ease_in_characters = false;
        }
    }

    if (s_ease_out_characters) {

        float progress = s_ease_out(&s_ease_out_timer_characters, &s_offset_y_characters, 0, -360);

        if (progress >= 1) {
            s_ease_out_timer_characters = 0;
            s_ease_out_characters = false;
            m_draw_characters = false;
        }
    }

    if (s_ease_in_settings) { 
        float progress = s_ease_out(&s_ease_out_timer_settings, &s_offset_y_settings, 360, 0);

        if (progress >= 1) {
            s_ease_out_timer_settings = 0;
            s_ease_in_settings = false;
        }
    }

    if (s_ease_out_settings) {

        float progress = s_ease_out(&s_ease_out_timer_settings, &s_offset_y_settings, 0, -360);

        if (progress >= 1) {
            s_ease_out_timer_settings = 0;
            s_ease_out_settings = false;
            m_draw_settings = false;
        }
    }
    
    if (s_ease_in_about) { 
        float progress = s_ease_out(&s_ease_out_timer_about, &s_offset_y_about, 360, 0);

        if (progress >= 1) {
            s_ease_out_timer_about = 0;
            s_ease_in_about = false;
        }
    }

    if (s_ease_out_about) {

        float progress = s_ease_out(&s_ease_out_timer_about, &s_offset_y_about, 0, -360);

        if (progress >= 1) {
            s_ease_out_timer_about = 0;
            s_ease_out_about = false;
            m_draw_about = false;
        }
    }
    

    if (m_draw_characters) {
        draw_characters();
    }
    if (m_draw_settings) { 
        draw_settings();
    }
    if (m_draw_about) { 
        draw_about();
    }

    clear_ui();
}