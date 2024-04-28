#include "screens.h"
#include "t_sprite.h"
#include "t_ui.h"

#include "stdio.h"
#include "stdbool.h"

#include "t_engine.h"

#define CC_LIGHT_RED (t_color) { 242, 97, 63, 255 }
#define CC_RED (t_color) { 155, 57, 34, 255 }
#define CC_DARK_RED (t_color) { 72, 30, 20, 255 }
#define CC_BLACK (t_color) { 12, 12, 12, 255 }


// EXTERN
extern const int SCREEN_WIDTH_DEFAULT;
extern const int SCREEN_HEIGHT_DEFAULT;

extern bool m_should_change_screen;
extern t_screen m_should_change_screen_to;

// SPRITES
static t_sprite m_button_sprite;
static t_sprite m_dropdown_sprite;
static t_sprite m_slider_background_sprite;

// UI
//  BUTTONS
static t_ui_button m_start_button;
static t_ui_button m_settings_button;
static t_ui_button m_quit_button;
static t_ui_button m_begin_button;
static t_ui_button m_edit_button;

//  OTHER
static t_ui_dropdown m_dropdown;
static char* dropdown_otions[] = { "Windowed", "Full-screen", "Borderless", "Full-screen Borderless" };


// BUTTON CALLBACKS

static void on_button_mouse_enter() {
    t_play_audio("./res/audio/click_003.wav");
}

static void on_start_button_clicked() {
    printf("start clicked\n");
    m_should_change_screen = true;
    m_should_change_screen_to = GAME;
    t_play_audio("./res/audio/click_003.wav");
}

static void on_settings_button_clicked() {
    printf("settings clicked\n");
    t_play_audio("./res/audio/click_003.wav");
}

static void on_quit_button_clicked() {
    printf("quit clicked\n");
    t_play_audio("./res/audio/click_003.wav");
}

void load_title_screen() {

    create_sprite("./res/textures/panel-border-030.png", &m_dropdown_sprite);
    m_dropdown_sprite.slice_borders = (t_vec4){ 16, 16, 16, 16 };

    create_sprite("./res/textures/panel-transparent-center-008.png", &m_button_sprite);
    m_button_sprite.slice_borders = (t_vec4){ 16, 16, 16, 16 };

    m_start_button = create_ui_button(&m_button_sprite, (t_rect) { 64, 92, 128, 48 });
    m_start_button.color_default = CC_LIGHT_RED;
    m_start_button.on_released = on_start_button_clicked;
    m_start_button.on_mouse_enter = on_button_mouse_enter;

    m_settings_button = create_ui_button(&m_button_sprite, (t_rect) { 64, 140 + 16, 128, 48 });
    m_settings_button.color_default = CC_LIGHT_RED;
    m_settings_button.on_released = on_settings_button_clicked;
    m_settings_button.on_mouse_enter = on_button_mouse_enter;

    m_quit_button = create_ui_button(&m_button_sprite, (t_rect) { 64, 188 + 38, 128, 48});
    m_quit_button.color_default = CC_LIGHT_RED;
    m_quit_button.on_released = on_quit_button_clicked;
    m_quit_button.on_mouse_enter = on_button_mouse_enter;

    m_begin_button = create_ui_button(&m_button_sprite, (t_rect) { 368, 292, 112, 40});
    m_begin_button.color_default = CC_BLACK;
    m_begin_button.color_mouseover = CC_DARK_RED;
    m_begin_button.color_clicked = CC_RED;
    // m_begin_button.on_released = on_quit_button_clicked;
    m_begin_button.on_mouse_enter = on_button_mouse_enter;

    m_edit_button = create_ui_button(&m_button_sprite, (t_rect) { 496, 292, 112, 40});
    m_edit_button.color_default = CC_BLACK;
    m_edit_button.color_mouseover = CC_DARK_RED;
    m_edit_button.color_clicked = CC_RED;
    // m_edit_button.on_released = on_quit_button_clicked;
    m_edit_button.on_mouse_enter = on_button_mouse_enter;

    m_dropdown = create_ui_dropdown(&m_dropdown_sprite, (t_rect) { 128, 16 + 64 + 16, 144, 48 });

    m_dropdown.options = dropdown_otions;
    m_dropdown.options_count = 4;

    create_sprite("./res/textures/slider_background.png", &m_slider_background_sprite);
    m_slider_background_sprite.slice_borders = (t_vec4){ 16, 16, 0, 0 };

}

void unload_title_screen() {

}

void update_title_screen() {

}

void draw_title_screen() {

    clear_color((t_color) { 12, 12, 12, 255 });

    // LEFT SIDE BUTTONS
    draw_ui_button(&m_start_button);
    draw_ui_button(&m_settings_button);
    draw_ui_button(&m_quit_button);

    // RIGHT SIDE BACKGROUND
    draw_sprite(&m_button_sprite, 256, 16, 368, 328, CC_LIGHT_RED);

    // CHARACTER ELEMENTS
    draw_sprite(&m_button_sprite, 272, 32, 128, 223, CC_BLACK);
    draw_sprite(&m_button_sprite, 272 + 128 + 16, 32, 128, 223, CC_BLACK);
    draw_sprite(&m_button_sprite, 272 + 128 + 16 + 128 + 16, 32, 128, 223, CC_BLACK);

    // SLIDER
    draw_sprite(&m_slider_background_sprite, 272, 328 - 64, 336, 16, CC_BLACK);

    // CHARACTER BUTTONS    
    draw_ui_button(&m_begin_button);
    draw_ui_button(&m_edit_button);

    clear_ui();
}