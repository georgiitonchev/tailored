#include "tailored.h"
#include <stdio.h>

// EXTERN
extern t_input_state input_state;
extern t_rect clip_areas[2];

// STATIC
static int m_currently_over_ui = 0;
static t_rect s_ui_raycast_block = { 0 };

t_ui_button create_ui_button(t_sprite* sprite) {

    t_ui_button ui_button;

    ui_button.user_data = NULL;

    ui_button.font = NULL;
    ui_button.text = NULL;

    ui_button.sprite = sprite;
    ui_button.color_default = WHITE;
    ui_button.color_mouseover = LIGHT_GRAY;
    ui_button.color_clicked = DARK_GRAY;
    ui_button.color_disabled = DARK_GRAY;

    ui_button.color_text_default = BLACK;
    ui_button.color_text_mouseover = DARK_GRAY;
    ui_button.color_text_clicked = LIGHT_GRAY;
    ui_button.color_text_disabled = DARK_GRAY;

    ui_button.is_mouse_over = false;
    ui_button.was_clicked = false;
    ui_button.is_selected = false;
    ui_button.is_disabled = false;

    ui_button.on_released = NULL;
    ui_button.on_mouse_enter = NULL;
    ui_button.on_mouse_exit = NULL;
    ui_button.on_pressed = NULL;
    ui_button.mouse_clicked_at = VEC2_ZERO;

    return ui_button;
}


t_ui_button create_ui_button_t(t_font* font, const char* text) { 

    t_ui_button ui_button;

    ui_button.user_data = NULL;

    ui_button.font = font;
    ui_button.text = text;

    ui_button.sprite = NULL;
    ui_button.color_default = WHITE;
    ui_button.color_mouseover = LIGHT_GRAY;
    ui_button.color_clicked = DARK_GRAY;
    ui_button.color_disabled = DARK_GRAY;

    ui_button.color_text_default = BLACK;
    ui_button.color_text_mouseover = DARK_GRAY;
    ui_button.color_text_clicked = LIGHT_GRAY;
    ui_button.color_text_disabled = DARK_GRAY;

    ui_button.is_mouse_over = false;
    ui_button.was_clicked = false;
    ui_button.is_selected = false;
    ui_button.is_disabled = false;

    ui_button.on_released = NULL;
    ui_button.on_mouse_enter = NULL;
    ui_button.on_mouse_exit = NULL;
    ui_button.on_pressed = NULL;
    ui_button.mouse_clicked_at = VEC2_ZERO;

    return ui_button;
}

void draw_ui_button(t_ui_button* button, int x, int y, int width, int height) {

    if (clip_areas[0].width + clip_areas[0].height > 0) {
        if (!does_rect_overlap_rect((t_rect){x, y, width, height}, clip_areas[0]))
        return;
    }

    t_color color = button->color_default;
    t_color color_text = button->color_text_default;

    if (button->is_disabled) { 
        color = button->color_disabled;
        color_text = button->color_text_disabled;
    }
    else if (is_rect_zero(s_ui_raycast_block) || !is_point_in_rect(mouse_position(), s_ui_raycast_block) ){ 
        bool is_mouse_inside_clip_area =
            clip_areas[0].width + clip_areas[0].height > 0 ? 
            is_point_in_rect(input_state.mouse_state.position, clip_areas[0]) : true;

        if (is_mouse_inside_clip_area && is_point_in_rect(input_state.mouse_state.position, (t_rect){ x, y, width, height }))
        {
            if (!button->is_mouse_over) {
                button->is_mouse_over = true;

                if (button->on_mouse_enter)
                    button->on_mouse_enter();
            }

            color = button->color_mouseover;
            color_text = button->color_text_mouseover;

            if (!button->was_clicked && is_mouse_button_pressed(MOUSE_BUTTON_LEFT)) {
                button->was_clicked = true;
                button->mouse_clicked_at = t_vec2_sub(input_state.mouse_state.position, (t_vec2){ x, y });
            }

            if (button->was_clicked) {
                color = button->color_clicked;
                color_text = button->color_text_clicked;
            }

            if (is_mouse_button_released(MOUSE_BUTTON_LEFT))
                if (button->was_clicked && button->on_released)
                    button->on_released(button);
        }
        else {
            if (button->is_mouse_over && button->on_mouse_exit)
                button->on_mouse_exit();

            button->is_mouse_over = false;
        }

        if (is_mouse_button_released(MOUSE_BUTTON_LEFT))
            button->was_clicked = false;

        if (button->was_clicked && button->on_pressed) {
            button->on_pressed(button);
        }

        if (button->is_selected) {
            color = button->color_clicked;
            color_text = button->color_text_clicked;
        }
    }

    if (button->sprite != NULL) {
        t_draw_sprite(button->sprite, x, y, width, height, color);
    }

    if (button->text != NULL && button->font != NULL) {

        t_vec2 text_size = measure_text_size_ttf(button->text, button->font);
        draw_text_ttf(button->text, button->font, (t_vec2) { x + (width - text_size.x) / 2, y + (height + text_size.y ) / 2}, color_text, 0);
    }
}

t_ui_dropdown create_ui_dropdown(t_sprite* sprite, t_rect rect) {

    t_ui_dropdown dropdown;

    dropdown.text = "Default";
    dropdown.sprite = sprite;
    dropdown.rect = rect;
    dropdown.color_default = WHITE;
    dropdown.color_mouseover = LIGHT_GRAY;
    dropdown.color_clicked = DARK_GRAY;

    // dropdown.mouse_entered = false;
    dropdown.was_clicked = false;
    dropdown.is_selected = false;

    // dropdown.on_released = NULL;
    // dropdown.on_mouse_enter = NULL;
    // dropdown.on_mouse_exit = NULL;
    // dropdown.on_pressed = NULL;

    return dropdown;
}

void draw_ui_dropdown(t_ui_dropdown* dropdown) {
    UNUSED(dropdown);
    // t_color color = dropdown->color_default;

    // if (/*!is_point_in_rect(global_state.mouse_pos, m_focused_rect) && !m_currently_over_ui &&*/ is_point_in_rect(input_state.mouse_state.position, dropdown->rect))
    // {
    //     m_currently_over_ui = true;
    //     color = dropdown->color_mouseover;

    //     if (is_mouse_button_pressed(MOUSE_BUTTON_LEFT)) {
    //         dropdown->was_clicked = true;
    //     }

    //     if (dropdown->was_clicked)
    //         color = dropdown->color_clicked;

    //     if (dropdown->was_clicked && is_mouse_button_released(MOUSE_BUTTON_LEFT)) {

    //         dropdown->is_selected = !dropdown->is_selected;
    //         m_focused_rect = (t_rect){ 0 };
    //     }
    // }

    // if (dropdown->is_selected) {
    //     color = dropdown->color_clicked;

    //     t_rect optionsRect = {
    //         dropdown->rect.x,
    //         dropdown->rect.y + dropdown->rect.height - 1,
    //         dropdown->rect.width,
    //         dropdown->rect.height * dropdown->options_count };

    //     m_focused_rect = optionsRect;

    //     draw_sprite_t(dropdown->sprite, optionsRect, color);
    //     // DrawRectangle(optionsRect.x, optionsRect.y, optionsRect.width, optionsRect.height, (t_color) {32, 32, 32, 255});
    //     // DrawRectangleLinesEx(optionsRect, 1, dropdown->color_clicked);

    //     for (int i = 0; i < dropdown->options_count; i++)
    //     {
    //         t_rect optionRect = { dropdown->rect.x + 1, dropdown->rect.y + dropdown->rect.height + dropdown->rect.height * i , dropdown->rect.width - 2, dropdown->rect.height - 2 };

    //         if (is_point_in_rect(input_state.mouse_state.position, optionRect))
    //         {
    //             m_currently_over_ui = 1;
    //             // DrawRectangle(optionRect.x, optionRect.y, optionRect.width, optionRect.height, (t_color) { 48, 48, 48, 255 });

    //             if (is_mouse_button_released(MOUSE_BUTTON_LEFT)) {
    //                 dropdown->on_option_selected(i);
    //             }
    //         }

    //         char* option = dropdown->options[i];
    //          t_vec2 text_size = measure_text_size(option, 16);
    //          draw_text(option, (t_vec2) { dropdown->rect.x + 16, dropdown->rect.y + dropdown->rect.height + dropdown->rect.height * i + (dropdown->rect.height - text_size.y) / 2}, 16, WHITE);
    //     }
    // }

    // if (is_mouse_button_released(MOUSE_BUTTON_LEFT)) {
    //     dropdown->was_clicked = false;
    //     m_focused_rect = (t_rect){ 0 };
    // }

    // draw_sprite_t(dropdown->sprite, dropdown->rect, color);
    // // DrawRectangleLinesEx(dropdown->rect, 1, color);

    //  t_vec2 text_size = measure_text_size(dropdown->text, 16);
    //  draw_text(dropdown->text, (t_vec2) { dropdown->rect.x + 16, dropdown->rect.y + (dropdown->rect.height - text_size.y) / 2 }, 16, WHITE);
    // // DrawTriangle(
    //     (Vector2) { dropdown->rect.x + dropdown->rect.width - 16 - dropdown->rect.height / 3, dropdown->rect.y + dropdown->rect.height / 2 - (dropdown->rect.height / 3) / 2 },
    //     (Vector2) { dropdown->rect.x + dropdown->rect.width - 16 - (dropdown->rect.height / 3) / 2,  dropdown->rect.y + dropdown->rect.height / 2 - (dropdown->rect.height / 3) / 2 + (dropdown->rect.height / 3)},
    //     (Vector2) { dropdown->rect.x + dropdown->rect.width - 16, dropdown->rect.y + dropdown->rect.height / 2 - (dropdown->rect.height / 3) / 2 }, 
    //     WHITE);
}

void set_ui_raycast_block(int x, int y, int width, int height) {
    s_ui_raycast_block.x = x;
    s_ui_raycast_block.y = y;
    s_ui_raycast_block.width = width;
    s_ui_raycast_block.height = height;
}

void set_ui_raycast_block_r(t_rect rect) {
    set_ui_raycast_block(rect.x, rect.y, rect.width, rect.height);
}

void clear_ui() {
    m_currently_over_ui = false;
}