
#pragma once

#include "tailored.h"
#include "t_sprite.h"

typedef struct t_ui_button {
    t_sprite* sprite;

    t_color color_default;
    t_color color_mouseover;
    t_color color_clicked;
    t_color color_disabled;

    void (*on_released)(struct t_ui_button*);
    void (*on_mouse_enter)();
    void (*on_mouse_exit)();
    void (*on_pressed)();

    bool is_mouse_over;
    bool was_clicked;
    bool is_selected;
    bool is_disabled;

    void* user_data;

    t_vec2 mouse_clicked_at;

} t_ui_button;

typedef struct t_ui_dropdown {

    t_rect rect;
    t_sprite* sprite;

    const char* text;
    // Font font;
    // int fontSize;

    t_color color_default;
    t_color color_mouseover;
    t_color color_clicked;
    t_color color_disabled;

    bool was_clicked;
    bool is_selected;

    char** options;
    int options_count;

    void (*on_option_selected)(int);

} t_ui_dropdown;

typedef struct t_ui_padding {

    float left;
    float right;
    float top;
    float bottom;
} t_ui_padding;

t_ui_button create_ui_button(t_sprite* sprite);
void draw_ui_button(t_ui_button* button, int x, int y, int width, int height);

t_ui_dropdown create_ui_dropdown(t_sprite* sprite, t_rect rect);
void draw_ui_dropdown(t_ui_dropdown* dropdown);

void clear_ui();