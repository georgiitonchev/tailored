#include "screens.h"

#include "t_engine.h"
#include "t_sprite.h"
#include "t_ui.h"

#include "stdio.h"
#include "stdbool.h"

extern bool m_should_change_screen;
extern t_screen m_should_change_screen_to;

// SPRITES
static t_sprite m_button_sprite;

// UI
static t_ui_button m_quit_button;

static void on_button_mouse_enter() {
    t_play_audio("./res/audio/click_003.wav");
}

static void on_quit_button_clicked() {
    printf("quit clicked\n");
    m_should_change_screen = true;
    m_should_change_screen_to = TITLE;
}

void load_game_screen() {

    create_sprite("./res/textures/panel-border-030.png", &m_button_sprite);
    m_button_sprite.slice_borders = (t_vec4){ 16, 16, 16, 16 };

    m_quit_button = create_ui_button(&m_button_sprite, (t_rect) { 16, 16 + 64 + 16, 96, 32 });
    m_quit_button.on_released = on_quit_button_clicked;
    m_quit_button.on_mouse_enter = on_button_mouse_enter;
}

void unload_game_screen() {

}

void update_game_screen() {

}

void draw_game_screen() {
    clear_color(BLUE);
    draw_ui_button(&m_quit_button);
}