#include "screens.h"

#include "./engine/tailored.h"
#include "./engine/t_font.h"
#include "./engine/t_sprite.h"
#include "./engine/t_input.h"
#include "./engine/t_ui.h"

#include "game.h"

#include "stdio.h"
#include "stdlib.h"
#include "stdbool.h"

#include "cJSON.h"

// EXTERNS
extern bool m_should_change_screen;
extern t_screen m_should_change_screen_to;
extern char* g_save_file; 

// FONTS
static t_font s_ui_font_m;

// SPRITES
static t_sprite m_button_sprite;

// UI
static t_ui_button s_button_quit;
static t_ui_button s_button_save;

static t_ui_button s_button_position;

// STATE
static t_vec2 s_saved_position;
static cJSON* s_json_data;

static void s_on_button_position_pressed(t_ui_button* button) {
    s_saved_position = t_vec2_sub(mouse_position(), button->mouse_clicked_at);
}

static void s_on_button_quit_clicked() {

    m_should_change_screen = true;
    m_should_change_screen_to = TITLE;
}

static void s_on_button_save_clicked() {

    printf("Saving file.");

    cJSON* json_object_position = cJSON_GetObjectItem(s_json_data, "position");

    cJSON* json_object_position_x = cJSON_GetObjectItem(json_object_position, "x");
    json_object_position_x->valuedouble = s_saved_position.x;

    cJSON* json_object_position_y = cJSON_GetObjectItem(json_object_position, "y");
    json_object_position_y->valuedouble = s_saved_position.y;

    const char* json_string = cJSON_Print(s_json_data);
    t_result write_file_result = t_write_file(g_save_file, json_string);

    free(json_string);

    if (write_file_result != T_SUCCESS) { 
        printf("ERROR: Could not update save file - %s.\n", g_save_file);
    } else {
        printf("Save file updated successfuly - %s.\n", g_save_file);
    }
}

static void s_load_save_file() { 

    long file_size;
    const char* save_file_data = t_read_file(g_save_file, &file_size);
    if (save_file_data == NULL) {
        printf("ERROR: Could not open save file: %s", g_save_file);
        return;
    }

    s_json_data = cJSON_Parse(save_file_data);
    if (s_json_data == NULL) {
        printf("ERROR: Could not process save file: %s", g_save_file);
        return;
    }

    cJSON* json_object_position = cJSON_GetObjectItem(s_json_data, "position");
    if (json_object_position == NULL) {

        json_object_position = cJSON_AddObjectToObject(s_json_data, "position");
        cJSON_AddNumberToObject(json_object_position, "x", 0);
        cJSON_AddNumberToObject(json_object_position, "y", 0);
        return;
    }

    s_saved_position.x = cJSON_GetObjectItem(json_object_position, "x")->valuedouble;
    s_saved_position.y = cJSON_GetObjectItem(json_object_position, "y")->valuedouble;
}

void load_game_screen() {

    s_saved_position = VEC2_ZERO;

    s_ui_font_m = load_ttf_font("./res/fonts/Eczar-Regular.ttf", 34);

    create_sprite("./res/textures/panel-transparent-center-030.png", &m_button_sprite);
    m_button_sprite.slice_borders = (t_vec4){ 16, 16, 16, 16 };

    s_button_quit = create_ui_button(&m_button_sprite);
    s_button_quit.color_default = CC_LIGHT_RED;
    s_button_quit.color_disabled = CC_RED;
    s_button_quit.on_released = s_on_button_quit_clicked;

    s_button_save = create_ui_button(&m_button_sprite);
    s_button_save.color_default = CC_LIGHT_RED;
    s_button_save.color_disabled = CC_RED;
    s_button_save.on_released = s_on_button_save_clicked;

    s_button_position = create_ui_button(&m_button_sprite);
    s_button_position.color_default = CC_LIGHT_RED;
    s_button_position.color_disabled = CC_RED;
    s_button_position.on_pressed = s_on_button_position_pressed;

    s_load_save_file();
}

void unload_game_screen() {

    delete_sprite(&m_button_sprite);
    delete_ttf_font(&s_ui_font_m);

    cJSON_Delete(s_json_data);
}

void update_game_screen() {

}

void draw_game_screen() {
    t_clear_color(CC_BLACK);
    draw_ui_button(&s_button_position, s_saved_position.x, s_saved_position.y, 100, 100);

    draw_ui_button(&s_button_quit, 16, 16, 96, 32);
    t_vec2 size_text_quit = measure_text_size_ttf("Quit", &s_ui_font_m);
    draw_text_ttf("Quit", &s_ui_font_m, (t_vec2) { 16 + (96 - size_text_quit.x) / 2, 16 + (32 + size_text_quit.y) / 2}, CC_BLACK, 0);

    draw_ui_button(&s_button_save, 16, 56, 96, 32);
    t_vec2 size_text_save = measure_text_size_ttf("Save", &s_ui_font_m);
    draw_text_ttf("Save", &s_ui_font_m, (t_vec2) { 16 + (96 - size_text_save.x) / 2, 56 + (32 + size_text_save.y) / 2}, CC_BLACK, 0);
    clear_ui();
}