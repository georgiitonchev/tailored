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

#define CC_LIGHT_RED (t_color) { 242, 97, 63, 255 }
#define CC_RED (t_color) { 155, 57, 34, 255 }
#define CC_DARK_RED (t_color) { 72, 30, 20, 255 }
#define CC_BLACK (t_color) { 12, 12, 12, 255 }

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
static t_sprite m_dropdown_sprite;

static t_sprite m_slider_background_sprite;
static t_sprite m_slider_knob_small_sprite;
static t_sprite m_slider_knob_big_sprite;

static t_sprite m_slider_background_sprite_v;
static t_sprite m_slider_knob_small_sprite_v;
static t_sprite m_slider_knob_big_sprite_v;

// UI
//  BUTTONS
static t_ui_button m_characters_button;
static t_ui_button m_settings_button;
static t_ui_button m_about_button;

static t_ui_button m_begin_button;
static t_ui_button m_edit_button;

static t_ui_button s_new_button;

// SLIDER KNOB
static t_ui_button m_slider_knob_button;
static t_ui_button m_slider_knob_button_v;

//  OTHER
static t_ui_dropdown m_dropdown;
static char* dropdown_otions[] = { "Windowed", "Full-screen", "Borderless", "Full-screen Borderless" };

static t_font s_ui_font_l;
static t_font s_ui_font_m;
static t_font s_ui_font_s;

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

static t_list* m_characters_list;

// CHARACTERS SCROLL AREA THINGS
static float scroll_area_width = 416;


// BUTTON CALLBACKS

static void on_button_mouse_enter() {
     //t_play_audio("./res/audio/click_003.wav");
}


static t_ui_button* m_selected_character;

static void on_character_button_clicked(t_ui_button* button) {
    if (m_selected_character != button) {

        if (m_selected_character != NULL) {
            m_selected_character->sprite = &m_button_sprite;
            m_selected_character->is_selected = false;
        }

        button->sprite = &m_button_sprite_selected;
        button->is_selected = true;
        m_selected_character = button;
    } else {

        m_selected_character->sprite = &m_button_sprite;
        m_selected_character->is_selected = false;
        m_selected_character = NULL;
    }

    m_edit_button.is_disabled = m_selected_character == NULL;
    m_begin_button.is_disabled = m_selected_character == NULL;
}


static void on_button_new_clicked() {

    cJSON* json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "name", "Jorko");

    // cJSON* jsonColor = cJSON_AddObjectToObject(json, "color");
    // cJSON_AddNumberToObject(jsonColor, "red", modelColor.r);
    // cJSON_AddNumberToObject(jsonColor, "green", modelColor.g);
    // cJSON_AddNumberToObject(jsonColor, "blue", modelColor.b);
    // cJSON_AddNumberToObject(jsonColor, "alpha", modelColor.a);

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
    *character_button = create_ui_button(&m_button_sprite);

    character_button->color_default = CC_BLACK;
    character_button->color_mouseover = CC_DARK_RED;
    character_button->color_clicked = CC_RED;

    char* key = (char*) malloc(7 * sizeof(char)); // FILE 0
    sprintf(key, "File %d", 1);
    character_button->user_data = key;

    character_button->on_released = on_character_button_clicked;

    add_to_list(m_characters_list, character_button);

    scroll_area_width = m_characters_list->size * (128 + 16) - 16;
}

static void on_characters_button_clicked() {

    if (m_draw_characters || s_ease_in_about || s_ease_in_settings) return;

    m_characters_button.sprite = &m_button_sprite_selected;
    m_characters_button.is_disabled = true;

    m_settings_button.sprite = &m_button_sprite;
    m_settings_button.is_disabled = false;

    m_about_button.sprite = &m_button_sprite;
    m_about_button.is_disabled = false;

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
    m_characters_button.is_disabled = false;

    m_settings_button.sprite = &m_button_sprite_selected;
    m_settings_button.is_disabled = true;

    m_about_button.sprite = &m_button_sprite;
    m_about_button.is_disabled = false;

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
    m_characters_button.is_disabled = false;

    m_settings_button.sprite = &m_button_sprite;
    m_settings_button.is_disabled = false;

    m_about_button.sprite = &m_button_sprite_selected;
    m_about_button.is_disabled = true;

    if (m_draw_characters) {
        s_ease_out_characters = true;
    }
    else if (m_draw_settings) {
        s_ease_out_settings = true;
    }

    s_ease_in_about = true;
    m_draw_about = true;
}

static float scroll_area_offset = 0;

static t_vec2 slider_pos = { 272, 264 };
static float slider_width = 336;

static t_rect slider_knob_rect;

static void on_slider_knob_button_pressed() {
    slider_knob_rect.x = input_state.mouse_state.position.x - m_slider_knob_button.mouse_clicked_at.x;

    // limits
    if (slider_knob_rect.x < slider_pos.x)
        slider_knob_rect.x = slider_pos.x;
    else if (slider_knob_rect.x > slider_pos.x + slider_width - slider_knob_rect.width)
        slider_knob_rect.x = slider_pos.x + slider_width - slider_knob_rect.width;

    float normalized_knob_position = (slider_knob_rect.x - slider_pos.x) / (slider_width - slider_knob_rect.width);
    scroll_area_offset = (scroll_area_width - slider_width) * normalized_knob_position;
}

static float scroll_area_height_v = 700;
static float scroll_area_offset_v = 0;

static t_vec2 slider_pos_v = { 597, 32 };
static float slider_height_v = 296;

static t_rect slider_knob_rect_v;

static void on_slider_knob_button_v_pressed() {
    slider_knob_rect_v.y = input_state.mouse_state.position.y - m_slider_knob_button_v.mouse_clicked_at.y;

    // limits
    if (slider_knob_rect_v.y < slider_pos_v.y)
        slider_knob_rect_v.y = slider_pos_v.y;
    else if (slider_knob_rect_v.y > slider_pos_v.y + slider_height_v - slider_knob_rect_v.height)
        slider_knob_rect_v.y = slider_pos_v.y + slider_height_v - slider_knob_rect_v.height;

    float normalized_knob_position = (slider_knob_rect_v.y - slider_pos_v.y) / (slider_height_v - slider_knob_rect_v.height);
    scroll_area_offset_v = (scroll_area_height_v - slider_height_v) * normalized_knob_position;
}

static void draw_characters() {
    // RIGHT SIDE BACKGROUND
    draw_sprite(&m_button_sprite, 256, 16 + s_offset_y_characters, 368, 328, CC_LIGHT_RED);

    // CHARACTER ELEMENTS
    if (m_characters_list->size > 0) {

        t_begin_scissor(256 + 16, 16 + s_offset_y_characters, 368 - 32, 328);
        for (unsigned int i = 0; i < m_characters_list->size; i ++) {

            t_ui_button* character_button = (t_ui_button*)element_at_list(m_characters_list, i);
            draw_ui_button(character_button, (272 + i * 128 + i * 16) - scroll_area_offset, 32 + s_offset_y_characters, 128, 223);

            draw_rect((272 + i * 128 + i * 16) - scroll_area_offset + 4, 32 + s_offset_y_characters + 223 - 48, 120, 32, (t_color) {12, 12, 12, 100});
            //draw_rect_lines((272 + i * 128 + i * 16) - scroll_area_offset + 4, 32 + s_offset_y_characters + 223 - 48, 120, 32, (t_color) {12, 12, 12, 100});

            const char* character_key = (const char*)character_button->user_data;
            t_vec2 text_size = measure_text_size_ttf(character_key, &s_ui_font_s);
            draw_text_ttf(character_key, &s_ui_font_s, (t_vec2){ (272 + i * 128 + i * 16) - scroll_area_offset + (128 - text_size.x) / 2, 32 + s_offset_y_characters + 223 - 32 + text_size.y / 2}, CC_RED, 0);
        }
        t_end_scissor();

        t_rect slider_rect = (t_rect) { 272, 328 - 64 + s_offset_y_characters, 336, m_slider_background_sprite.texture.size.y };

        // BIG KNOB
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glClear(GL_STENCIL_BUFFER_BIT);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilMask(0xFF);
        draw_ui_button(&m_slider_knob_button, slider_knob_rect.x, slider_knob_rect.y + s_offset_y_characters, slider_knob_rect.width, slider_knob_rect.height);

        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        if (!m_slider_knob_button.is_mouse_over && is_point_in_rect(input_state.mouse_state.position, slider_rect)) {

            if (is_mouse_button_pressed(MOUSE_BUTTON_LEFT)) {
                slider_knob_rect.x = input_state.mouse_state.position.x - slider_knob_rect.width / 2;
                m_slider_knob_button.was_clicked = true;
                m_slider_knob_button.mouse_clicked_at = (t_vec2) { m_slider_knob_button.sprite->texture.size.x / 2, m_slider_knob_button.sprite->texture.size.y / 2};
            }

            t_rect slider_knob_small_rect = (t_rect) { input_state.mouse_state.position.x - m_slider_knob_small_sprite.texture.size.x / 2, slider_rect.y + s_offset_y_characters, m_slider_knob_small_sprite.texture.size.x, m_slider_knob_small_sprite.texture.size.y };

            //limits
            if (slider_knob_small_rect.x < slider_pos.x)
                slider_knob_small_rect.x = slider_pos.x;
            else if (slider_knob_small_rect.x > slider_pos.x + slider_width - slider_knob_small_rect.width)
                slider_knob_small_rect.x = slider_pos.x + slider_width - slider_knob_small_rect.width;

            draw_sprite_t(&m_slider_knob_small_sprite, slider_knob_small_rect, CC_BLACK);
        }

        glStencilMask(0x00);
        // SLIDER
        draw_sprite_t(&m_slider_background_sprite, slider_rect, CC_BLACK);

        glStencilMask(0xFF);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);

    } else {

        t_vec2 text_size_no_files_info = measure_text_size_ttf("Click on \"New\" to create a new save file.", &s_ui_font_s);
        draw_text_ttf("Click on \"New\" to create a new save file.", &s_ui_font_s, (t_vec2) { 256 + (368 - text_size_no_files_info.x) / 2 , 16 + s_offset_y_characters + (328 + text_size_no_files_info.y) / 2}, CC_BLACK, 0);

    }
    
    // CHARACTER BUTTONS
    draw_ui_button(&s_new_button, 256 + 16, 292 + s_offset_y_characters, 80, 40);
    t_vec2 text_size_new = measure_text_size_ttf("New", &s_ui_font_m);
    draw_text_ttf("New", &s_ui_font_m, (t_vec2) {256 + 16 + (80 - text_size_new.x) / 2 , 292 + s_offset_y_characters + (40 + text_size_new.y) / 2}, CC_RED, 0);

    draw_ui_button(&m_begin_button, 368, 292 + s_offset_y_characters, 112, 40);
    t_vec2 text_size_delete = measure_text_size_ttf("Delete", &s_ui_font_m);
    draw_text_ttf("Delete", &s_ui_font_m, (t_vec2) {368 + (112 - text_size_delete.x) / 2 , 292 + s_offset_y_characters + (40 + text_size_delete.y) / 2}, CC_RED, 0);

    draw_ui_button(&m_edit_button, 496, 292 + s_offset_y_characters, 112, 40);
    t_vec2 text_size_start = measure_text_size_ttf("Start", &s_ui_font_m);
    draw_text_ttf("Start", &s_ui_font_m, (t_vec2) {496 + (112 - text_size_start.x) / 2 , 292 + s_offset_y_characters + (40 + text_size_start.y) / 2}, CC_RED, 0);
}

static void draw_settings() {
    draw_sprite(&m_button_sprite, 256, 16 + s_offset_y_settings, 368, 328, CC_LIGHT_RED);

    t_vec2 text_size_sound = measure_text_size_ttf("Sound", &s_ui_font_l);
    draw_text_ttf("Sound", &s_ui_font_l, (t_vec2) {256 + (368 - text_size_sound.x) / 2, 48 + s_offset_y_settings}, CC_BLACK, 0);

    draw_text_ttf("Master", &s_ui_font_s, (t_vec2) {256 + 32, 48 + 24 + 8 + s_offset_y_settings}, CC_BLACK, 0);
    draw_sprite(&m_slider_background_sprite, 368,  48 + 22 + s_offset_y_settings, 176, m_slider_background_sprite.texture.size.y, CC_BLACK);
    draw_text_ttf("100", &s_ui_font_s, (t_vec2) { 368 + 176 + 16, 48 + 24 + 8 + s_offset_y_settings}, CC_BLACK, 0);

    draw_text_ttf("Music", &s_ui_font_s, (t_vec2) {256 + 32, 48 + 48 + 16 + s_offset_y_settings}, CC_BLACK, 0);
    draw_sprite(&m_slider_background_sprite, 368,  48 + 48 + 6 + s_offset_y_settings, 176, m_slider_background_sprite.texture.size.y, CC_BLACK);
    draw_text_ttf("100", &s_ui_font_s, (t_vec2) { 368 + 176 + 16, 48 + 48 + 16 + s_offset_y_settings}, CC_BLACK, 0);

    draw_text_ttf("Effects", &s_ui_font_s, (t_vec2) {256 + 32, 48 + 48 + 24 + 24 + s_offset_y_settings}, CC_BLACK, 0);
    draw_sprite(&m_slider_background_sprite, 368, 48 + 48 + 24 + 14 + s_offset_y_settings, 176, m_slider_background_sprite.texture.size.y, CC_BLACK);
    draw_text_ttf("100", &s_ui_font_s, (t_vec2) { 368 + 176 + 16, 48 + 48 + 24 + 24 + s_offset_y_settings}, CC_BLACK, 0);

}

static void draw_about() {
    draw_sprite(&m_button_sprite, 256, 16 + s_offset_y_about, 368, 328, CC_LIGHT_RED);

    t_begin_scissor(256 + 16, 32 + s_offset_y_about, 368 - 48, 328 - 32);
        draw_text_ttf("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Dictum sit amet justo donec. Faucibus nisl tincidunt eget nullam non. Egestas tellus rutrum tellus pellentesque eu tincidunt. Donec enim diam vulputate ut pharetra sit amet aliquam id. Quis viverra nibh cras pulvinar mattis nunc sed blandit. Posuere sollicitudin aliquam ultrices sagittis orci a scelerisque purus. In ornare quam viverra orci. Congue quisque egestas diam in arcu. Tempus urna et pharetra pharetra massa. Dolor magna eget est lorem ipsum dolor. Ut etiam sit amet nisl purus in mollis nunc sed. Cras fermentum odio eu feugiat pretium nibh ipsum consequat nisl. Consectetur purus ut faucibus pulvinar elementum. Elementum eu facilisis sed odio morbi.", &s_ui_font_s, (t_vec2) {256 + 16 , 16 + 32 + s_offset_y_about - scroll_area_offset_v}, CC_BLACK, 368 - 48);
    t_end_scissor();

    t_rect slider_rect = RECT_ZERO;
    slider_rect.x = 597;
    slider_rect.y = 32 + s_offset_y_about;
    slider_rect.width = m_slider_background_sprite_v.texture.size.x;
    slider_rect.height = 296;

    // BIG KNOB
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glClear(GL_STENCIL_BUFFER_BIT);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);
    draw_ui_button(&m_slider_knob_button_v, slider_knob_rect_v.x, slider_knob_rect_v.y + s_offset_y_about, slider_knob_rect_v.width, slider_knob_rect_v.height);

    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    if (!m_slider_knob_button_v.is_mouse_over && is_point_in_rect(input_state.mouse_state.position, slider_rect)) {

        if (is_mouse_button_pressed(MOUSE_BUTTON_LEFT)) {
            slider_knob_rect_v.y = input_state.mouse_state.position.y - slider_knob_rect_v.height / 2;
            m_slider_knob_button_v.was_clicked = true;
            m_slider_knob_button_v.mouse_clicked_at = (t_vec2) { m_slider_knob_button_v.sprite->texture.size.x / 2, m_slider_knob_button_v.sprite->texture.size.y / 2};
        }

        t_rect slider_knob_small_rect = (t_rect) { slider_rect.x, input_state.mouse_state.position.y - m_slider_knob_small_sprite_v.texture.size.y / 2 + s_offset_y_about, m_slider_knob_small_sprite_v.texture.size.x, m_slider_knob_small_sprite_v.texture.size.y };

        //limits
        if (slider_knob_small_rect.y < slider_rect.y)
            slider_knob_small_rect.y = slider_rect.y;
        else if (slider_knob_small_rect.y > slider_rect.y + slider_rect.height- slider_knob_small_rect.height)
            slider_knob_small_rect.y = slider_rect.y + slider_rect.height - slider_knob_small_rect.height;

        draw_sprite_t(&m_slider_knob_small_sprite_v, slider_knob_small_rect, CC_BLACK);
    }

    glStencilMask(0x00);
    // SLIDER
    draw_sprite_t(&m_slider_background_sprite_v, slider_rect, CC_BLACK);
    glStencilMask(0xFF);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
}

void load_title_screen() {

    s_ui_font_l = load_ttf_font("./res/fonts/Eczar-Regular.ttf", 42);
    s_ui_font_m = load_ttf_font("./res/fonts/Eczar-Regular.ttf", 37);
    s_ui_font_s = load_ttf_font("./res/fonts/Eczar-Regular.ttf", 32);

    create_sprite("./res/textures/imps_fairies_logo.png", &m_logo_sprite);

    create_sprite("./res/textures/panel-border-030.png", &m_dropdown_sprite);
    m_dropdown_sprite.slice_borders = (t_vec4){ 16, 16, 16, 16 };

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

    m_begin_button = create_ui_button(&m_button_sprite);
    m_begin_button.color_default = CC_BLACK;
    m_begin_button.color_mouseover = CC_DARK_RED;
    m_begin_button.color_clicked = CC_RED;
    m_begin_button.color_disabled = CC_DARK_RED;
    m_begin_button.is_disabled = true;
    // m_begin_button.on_released = on_quit_button_clicked;
    m_begin_button.on_mouse_enter = on_button_mouse_enter;

    m_edit_button = create_ui_button(&m_button_sprite);
    m_edit_button.color_default = CC_BLACK;
    m_edit_button.color_mouseover = CC_DARK_RED;
    m_edit_button.color_clicked = CC_RED;
    m_edit_button.color_disabled = CC_DARK_RED;
    m_edit_button.is_disabled = true;
    // m_edit_button.on_released = on_quit_button_clicked;
    m_edit_button.on_mouse_enter = on_button_mouse_enter;

    s_new_button = create_ui_button(&m_button_sprite);
    s_new_button.color_default = CC_BLACK;
    s_new_button.color_mouseover = CC_DARK_RED;
    s_new_button.color_clicked = CC_RED;
    s_new_button.color_disabled = CC_DARK_RED;
    s_new_button.on_mouse_enter = on_button_mouse_enter;
    s_new_button.on_released = on_button_new_clicked;

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

    slider_knob_rect = (t_rect){ 272, 328 - 66, m_slider_knob_big_sprite.texture.size.x, m_slider_knob_big_sprite.texture.size.y };

    m_characters_list = create_list(sizeof(t_ui_button));
    unsigned int save_files_count = 0;
    for (int i = 0; i < save_files_count; i++) {

        t_ui_button* character_button = (t_ui_button*)malloc(sizeof(t_ui_button));
        *character_button = create_ui_button(&m_button_sprite);

        character_button->color_default = CC_BLACK;
        character_button->color_mouseover = CC_DARK_RED;
        character_button->color_clicked = CC_RED;

        char* key = (char*) malloc(7 * sizeof(char)); // FILE 0
        sprintf(key, "File %d", i);
        character_button->user_data = key;

        character_button->on_released = on_character_button_clicked;

        add_to_list(m_characters_list, character_button);
    }

    scroll_area_width = m_characters_list->size * 128 + (m_characters_list->size - 1) * 16;

    // ABOUT
    create_sprite("./res/textures/slider_background_v.png", &m_slider_background_sprite_v);
    create_sprite("./res/textures/slider_knob_small_v.png", &m_slider_knob_small_sprite_v);
    create_sprite("./res/textures/slider_knob_big_v.png", &m_slider_knob_big_sprite_v);

    m_slider_knob_button_v = create_ui_button(&m_slider_knob_big_sprite_v);
    m_slider_knob_button_v.color_default = CC_BLACK;
    m_slider_knob_button_v.color_mouseover = CC_DARK_RED;
    m_slider_knob_button_v.color_clicked = CC_DARK_RED;
    m_slider_knob_button_v.on_pressed = on_slider_knob_button_v_pressed;

    //  slider_rect.x = 256 + 368 - 32 + 5;
    //  slider_rect.y = 32 + s_offset_y_about;
    slider_knob_rect_v = (t_rect){ 595, 32, m_slider_knob_big_sprite_v.texture.size.x, m_slider_knob_big_sprite_v.texture.size.y };

}

void unload_title_screen() {

}

void update_title_screen() {

}

void draw_title_screen() {

    t_clear_color(CC_BLACK);

    draw_sprite(&m_logo_sprite, 0, 0, m_logo_sprite.texture.size.x, m_logo_sprite.texture.size.y, CC_LIGHT_RED);

    // LEFT SIDE BUTTONS
    draw_ui_button(&m_characters_button, 64, 140, 128, 48);
    draw_ui_button(&m_settings_button, 64, 204, 128, 48);
    draw_ui_button(&m_about_button, 64, 268, 128, 48);

    t_vec2 text_size_play = measure_text_size_ttf("Play", &s_ui_font_l);
    draw_text_ttf("Play", &s_ui_font_l, (t_vec2) {64 + (128 - text_size_play.x) / 2 , 140 + (48 + text_size_play.y) / 2}, CC_BLACK, 0);

    t_vec2 text_size_settings = measure_text_size_ttf("Settings", &s_ui_font_l);
    draw_text_ttf("Settings", &s_ui_font_l, (t_vec2) {64 + (128 - text_size_settings.x) / 2, 204 + (48 + text_size_settings.y) / 2}, CC_BLACK, 0);

    t_vec2 text_size_about = measure_text_size_ttf("About", &s_ui_font_l);
    draw_text_ttf("About", &s_ui_font_l, (t_vec2) {64 + (128 - text_size_about.x) / 2 , 268 + (48 + text_size_about.y) / 2}, CC_BLACK, 0);

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
