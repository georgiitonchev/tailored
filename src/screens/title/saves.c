#include "screen_title.h"

#include "../../engine/tailored.h"
#include "../../engine/t_input.h"
#include "../../engine/t_sprite.h"
#include "../../engine/t_ui.h"
#include "../../engine/t_shapes.h"
#include "../../engine/t_font.h"
#include "../../engine/t_list.h"

#include "../../../dep/include/glad/glad.h"

#include "../../game.h"

#include "../../cJSON.h"

#include <stdio.h>
#include <stdlib.h>

// FONTS
static t_font s_font_ui_s;
static t_font s_font_ui_m;

// SPRITES
static t_sprite s_sprite_button;
static t_sprite s_sprite_button_selected;
static t_sprite s_sprite_section_background;
static t_sprite s_sprite_slider_background;
static t_sprite s_sprite_small_knob;
static t_sprite s_sprite_big_knob;

// BUTTONS
static t_ui_button s_button_slider_knob;
static t_ui_button s_button_new;
static t_ui_button s_button_delete;
static t_ui_button s_button_load;

// STATE
static t_list* s_list_saves;
static float s_scroll_area_offset;
static t_ui_button* s_selected_save;

static void s_on_save_file_clicked(t_ui_button* button) {
    if (s_selected_save != button) {

        if (s_selected_save != NULL) {
            s_selected_save->sprite = &s_sprite_button;
            s_selected_save->is_selected = false;
        }

        button->sprite = &s_sprite_button_selected;
        button->is_selected = true;
        s_selected_save = button;

    } else {

        s_selected_save->sprite = &s_sprite_button;
        s_selected_save->is_selected = false;
        s_selected_save = NULL;
    }

    s_button_delete.is_disabled = s_selected_save == NULL;
    s_button_load.is_disabled = s_selected_save == NULL;
}

static void s_on_button_new_clicked() {

    cJSON* json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "name", "Jorko");

    const char* json_string = cJSON_Print(json);
    const char* file_name = "saves/save_file.ass";
    // bool saved = SaveFileText(TextFormat("characters/%s.ass", _nameInputField.text), jsonString);
    FILE *file = fopen(file_name, "wt");
    if (file != NULL)
    {
        int count = fprintf(file, "%s", json_string);

        if (count < 0) printf("FILEIO: [%s] Failed to write text file.\n", file_name);
        else printf("FILEIO: [%s] Text file saved successfully.\n", file_name);

        fclose(file);
    }
    else {
        printf("Could not create file.\n");
    }

    t_ui_button* character_button = (t_ui_button*)malloc(sizeof(t_ui_button));
    *character_button = create_ui_button(&s_sprite_button);

    character_button->color_default = CC_BLACK;
    character_button->color_mouseover = CC_DARK_RED;
    character_button->color_clicked = CC_RED;

    char* key = (char*) malloc(7 * sizeof(char)); // FILE 0
    sprintf(key, "File %d", 1);
    character_button->user_data = key;

    character_button->on_released = s_on_save_file_clicked;

    add_to_list(s_list_saves, character_button);

    // scroll_area_width = m_characters_list->size * (128 + 16) - 16;
}

void load_section_saves() {

    s_font_ui_s = load_ttf_font("./res/fonts/Eczar-Regular.ttf", 32);
    s_font_ui_m = load_ttf_font("./res/fonts/Eczar-Regular.ttf", 37);

    create_sprite("./res/textures/panel-transparent-center-030.png", &s_sprite_button);
    s_sprite_button.slice_borders = (t_vec4){ 16, 16, 16, 16 };

    create_sprite("./res/textures/panel-transparent-center-030.png", &s_sprite_section_background);
    s_sprite_section_background.slice_borders = (t_vec4){ 16, 16, 16, 16 };

    create_sprite("./res/textures/slider_background.png", &s_sprite_slider_background);
    create_sprite("./res/textures/slider_knob_small.png", &s_sprite_small_knob);
    create_sprite("./res/textures/slider_knob_big.png", &s_sprite_big_knob);

    s_button_slider_knob = create_ui_button(&s_sprite_big_knob);
    s_button_slider_knob.color_default = CC_BLACK;
    s_button_slider_knob.color_mouseover = CC_DARK_RED;
    s_button_slider_knob.color_clicked = CC_DARK_RED;
    // s_button_slider_knob.on_pressed = s_on_slider_knob_button_pressed;

    s_button_new = create_ui_button(&s_sprite_button);
    s_button_new.color_default = CC_BLACK;
    s_button_new.color_mouseover = CC_DARK_RED;
    s_button_new.color_clicked = CC_RED;
    s_button_new.color_disabled = CC_DARK_RED;
    //m_begin_button.on_released = on_button_start_cicked;
    // s_button_new.on_mouse_enter = on_button_mouse_enter;

    s_button_delete = create_ui_button(&s_sprite_button);
    s_button_delete.color_default = CC_BLACK;
    s_button_delete.color_mouseover = CC_DARK_RED;
    s_button_delete.color_clicked = CC_RED;
    s_button_delete.color_disabled = CC_DARK_RED;
    s_button_delete.is_disabled = true;
    // s_button_delete.on_released = on_button_start_cicked;
    // s_button_delete.on_mouse_enter = on_button_mouse_enter;

    s_button_load = create_ui_button(&s_sprite_button);
    s_button_load.color_default = CC_BLACK;
    s_button_load.color_mouseover = CC_DARK_RED;
    s_button_load.color_clicked = CC_RED;
    s_button_load.color_disabled = CC_DARK_RED;
    s_button_load.is_disabled = true;
    // s_button_load.on_mouse_enter = on_button_mouse_enter;
    // s_button_load.on_released = on_button_new_clicked;

    s_list_saves = create_list(sizeof(t_ui_button));
}

void draw_section_saves(const float p_offset_x, const float p_offset_y) {

    draw_sprite(&s_sprite_section_background, 256 + p_offset_x, 16 + p_offset_y, 368, 328, CC_LIGHT_RED);

    // CHARACTER ELEMENTS
    if (s_list_saves->size > 0) {

        t_begin_scissor(256 + 16 + p_offset_x, 16 + p_offset_y, 368 - 32, 328);
        for (unsigned int i = 0; i < s_list_saves->size; i ++) {

            t_ui_button* character_button = (t_ui_button*)element_at_list(s_list_saves, i);
            draw_ui_button(character_button, (272 + i * 128 + i * 16) - s_scroll_area_offset + p_offset_x, 32 + p_offset_y, 128, 223);

            draw_rect((272 + i * 128 + i * 16) - s_scroll_area_offset + 4 + p_offset_x, 32 + p_offset_y + 223 - 48, 120, 32, (t_color) {12, 12, 12, 100});

            const char* character_key = (const char*)character_button->user_data;
            t_vec2 text_size = measure_text_size_ttf(character_key, &s_font_ui_s);
            draw_text_ttf(character_key, &s_font_ui_s, (t_vec2){ (272 + i * 128 + i * 16) - s_scroll_area_offset + (128 - text_size.x) / 2 + p_offset_x, 32 + p_offset_y + 223 - 32 + text_size.y / 2}, CC_RED, 0);
        }
        t_end_scissor();

        t_rect slider_rect = (t_rect) { 272 + p_offset_x, 328 - 64 + p_offset_y, 336, s_sprite_slider_background.texture.size.y };

        // BIG KNOB
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glClear(GL_STENCIL_BUFFER_BIT);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilMask(0xFF);
        draw_ui_button(&s_button_slider_knob,  272 + p_offset_x, 328 - 66 + p_offset_y, s_sprite_big_knob.texture.size.x, s_sprite_big_knob.texture.size.y);

        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        if (!s_button_slider_knob.is_mouse_over && is_point_in_rect(mouse_position(), slider_rect)) {

            // if (is_mouse_button_pressed(MOUSE_BUTTON_LEFT)) {
            //     slider_knob_rect.x = mouse_position().x - slider_knob_rect.width / 2;
            //     m_slider_knob_button.was_clicked = true;
            //     m_slider_knob_button.mouse_clicked_at = (t_vec2) { m_slider_knob_button.sprite->texture.size.x / 2, m_slider_knob_button.sprite->texture.size.y / 2};
            // }

            t_rect rect_small_knob = (t_rect) { mouse_position().x - s_sprite_small_knob.texture.size.x / 2 + p_offset_x, slider_rect.y + p_offset_y, s_sprite_small_knob.texture.size.x, s_sprite_small_knob.texture.size.y };

            // //limits
            // if (slider_knob_small_rect.x < slider_pos.x)
            //     slider_knob_small_rect.x = slider_pos.x;
            // else if (slider_knob_small_rect.x > slider_pos.x + slider_width - slider_knob_small_rect.width)
            //     slider_knob_small_rect.x = slider_pos.x + slider_width - slider_knob_small_rect.width;

            draw_sprite_t(&s_sprite_small_knob, rect_small_knob, CC_BLACK);
        }

        glStencilMask(0x00);
        // SLIDER
        draw_sprite_t(&s_sprite_slider_background, slider_rect, CC_BLACK);

        glStencilMask(0xFF);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);

    } else {

        t_vec2 text_size_no_files_info = measure_text_size_ttf("Click \"New\" to create a new save file.", &s_font_ui_s);
        draw_text_ttf("Click \"New\" to create a new save file.", &s_font_ui_s, (t_vec2) { 256 + (368 - text_size_no_files_info.x) / 2 + p_offset_x, 16 + p_offset_y + (328 + text_size_no_files_info.y) / 2}, CC_BLACK, 0);
    }

    // CHARACTER BUTTONS
    draw_ui_button(&s_button_new, 256 + 16 + p_offset_x, 292 + p_offset_y, 80, 40);
    t_vec2 text_size_new = measure_text_size_ttf("New", &s_font_ui_m);
    draw_text_ttf("New", &s_font_ui_m, (t_vec2) {256 + 16 + (80 - text_size_new.x) / 2  + p_offset_x, 292 + p_offset_y + (40 + text_size_new.y) / 2}, CC_RED, 0);

    draw_ui_button(&s_button_delete, 368 + p_offset_x, 292 + p_offset_y, 112, 40);
    t_vec2 text_size_delete = measure_text_size_ttf("Delete", &s_font_ui_m);
    draw_text_ttf("Delete", &s_font_ui_m, (t_vec2) {368 + (112 - text_size_delete.x) / 2  + p_offset_x, 292 + p_offset_y + (40 + text_size_delete.y) / 2}, CC_RED, 0);

    draw_ui_button(&s_button_load, 496 + p_offset_x, 292 + p_offset_y, 112, 40);
    t_vec2 text_size_start = measure_text_size_ttf("Load", &s_font_ui_m);
    draw_text_ttf("Load", &s_font_ui_m, (t_vec2) {496 + (112 - text_size_start.x) / 2 + p_offset_x, 292 + p_offset_y + (40 + text_size_start.y) / 2}, CC_RED, 0);

}

void unload_section_saves() {

}
