#include "t_ui.h"
#include "t_input.h"
#include "t_shapes.h"
#include "t_sprite.h"

extern t_global_state global_state;

void draw_ui_button(t_ui_btn* button) {

    t_color color = button->color_default;

    if (is_point_in_rect(global_state.mouse_pos, button->rect))
    {
        color = button->color_mouseover;

        if (is_mouse_button_pressed(MOUSE_BUTTON_LEFT))
            button->was_clicked_ptr = 1;

        if (button->was_clicked_ptr)
            color = button->color_clicked;

        if (is_mouse_button_released(MOUSE_BUTTON_LEFT))
            if (button->was_clicked_ptr && button->on_clicked)
                button->on_clicked(button);
    }

    if (is_mouse_button_released(MOUSE_BUTTON_LEFT))
        button->was_clicked_ptr = 0;

    if (button->is_selected_ptr)
        color = button->color_clicked;

    button->sprite->color = color;

    draw_sprite_t(button->sprite, button->rect);
}