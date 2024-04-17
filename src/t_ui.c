#include "t_ui.h"
#include "t_input.h"
#include "t_shapes.h"
#include "t_sprite.h"

extern t_global_state global_state;

t_ui_btn create_ui_button(t_sprite* sprite, t_rect rect) {

    t_ui_btn ui_button = {};

    ui_button.sprite = sprite;
    ui_button.rect = rect;
    ui_button.color_default = WHITE;
    ui_button.color_mouseover = LIGHT_GRAY;
    ui_button.color_clicked = DARK_GRAY;

    ui_button.mouse_entered = false;
    ui_button.was_clicked = false;
    ui_button.is_selected = false;

    return ui_button;
}

void draw_ui_button(t_ui_btn* button) {

    t_color color = button->color_default;

    if (is_point_in_rect(global_state.mouse_pos, button->rect))
    {
        if (!button->mouse_entered) {
            button->mouse_entered = true;

            if (button->on_mouse_enter)
                button->on_mouse_enter();
        }

        color = button->color_mouseover;

        if (is_mouse_button_pressed(MOUSE_BUTTON_LEFT))
            button->was_clicked = 1;

        if (button->was_clicked)
            color = button->color_clicked;

        if (is_mouse_button_released(MOUSE_BUTTON_LEFT))
            if (button->was_clicked && button->on_released)
                button->on_released(button);
    }
    else {
        if (button->mouse_entered && button->on_mouse_exit)
            button->on_mouse_exit();

        button->mouse_entered = false;
    }

    if (is_mouse_button_released(MOUSE_BUTTON_LEFT))
        button->was_clicked = 0;

    if (button->is_selected)
        color = button->color_clicked;

    button->sprite->color = color;

    draw_sprite_t(button->sprite, button->rect);
}