#include "screens.h"

#include "./engine/tailored.h"
#include "./engine/t_font.h"
#include "./engine/t_sprite.h"
#include "./engine/t_input.h"
#include "./engine/t_shapes.h"
#include "./engine/t_ui.h"

#include "game.h"

#include "stdio.h"
#include "stdlib.h"
#include "stdbool.h"

#include "cJSON.h"

// EXTERNS
extern char* g_save_file; 

// FONTS
static t_font s_font_ui_m;

// SPRITES
static t_sprite s_sprite_button;
static t_sprite s_sprite_spritesheet_idle;

// UI
static t_ui_button s_button_quit;
static t_ui_button s_button_save;

static t_ui_button s_button_quit_yes;
static t_ui_button s_button_quit_no;

static t_ui_button s_button_position;

// STATE
static t_vec2 s_saved_position;
static cJSON* s_json_data;

static bool s_has_saved = true;
static bool s_prompt_save = false;

static void s_on_button_position_pressed(t_ui_button* button) {
    s_saved_position = t_vec2_sub(mouse_position(), button->mouse_clicked_at);
    s_has_saved = false;
}

static void s_on_button_quit_yes_clicked() {

    set_screen(TITLE);
    s_prompt_save = false;
}

static void s_on_button_quit_no_clicked() {
    s_prompt_save = false;
}

static void s_on_button_quit_clicked() {

    if (s_has_saved) { 
        set_screen(TITLE);
    } else {
        s_prompt_save = true;
    }
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

    free((char *)json_string);

    if (write_file_result != T_SUCCESS) { 
        printf("ERROR: Could not update save file - %s.\n", g_save_file);
    } else {
        printf("Save file updated successfuly - %s.\n", g_save_file);
    }

    s_has_saved = true;
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

int load_game_screen(void* args) {
    UNUSED(args);

    t_load_texture_data_s(&s_sprite_button, "./res/textures/panel-transparent-center-030.png");
    t_load_texture_data_s(&s_sprite_spritesheet_idle, "./res/textures/spritesheet_imp_idle.png");

    s_load_save_file();

    set_loading_progress(1);
    set_loading_finished();
    return 0;
}

void init_game_screen() {

    s_font_ui_m = load_ttf_font("./res/fonts/Eczar-Regular.ttf", 34);

    s_has_saved = true;
    s_prompt_save = false;
    s_saved_position = VEC2_ZERO;

    t_init_sprite(&s_sprite_button);
    s_sprite_button.slice_borders = (t_vec4){ 16, 16, 16, 16 };

    s_button_quit = create_ui_button(&s_sprite_button);
    s_button_quit.color_default = CC_LIGHT_RED;
    s_button_quit.color_disabled = CC_RED;
    s_button_quit.on_released = s_on_button_quit_clicked;

    s_button_save = create_ui_button(&s_sprite_button);
    s_button_save.color_default = CC_LIGHT_RED;
    s_button_save.color_disabled = CC_RED;
    s_button_save.on_released = s_on_button_save_clicked;

    s_button_quit_yes = create_ui_button_t(&s_font_ui_m, "Yes");
    s_button_quit_yes.on_released = s_on_button_quit_yes_clicked;

    s_button_quit_no = create_ui_button_t(&s_font_ui_m, "No");
    s_button_quit_no.on_released = s_on_button_quit_no_clicked;

    // 200 x 350
    t_init_sprite(&s_sprite_spritesheet_idle);
    s_sprite_spritesheet_idle.texture_slice = (t_vec4) { 0, 0, 200, 360};

    s_button_position = create_ui_button(&s_sprite_spritesheet_idle);
    s_button_position.color_default = WHITE;
    s_button_position.on_pressed = s_on_button_position_pressed;
}

void unload_game_screen() {
    t_deinit_sprite(&s_sprite_button);
    delete_ttf_font(&s_font_ui_m);

    cJSON_Delete(s_json_data);
}

static float s_timer_animation = 0;
static int s_index_animation = 1;

void draw_game_screen() {
    t_clear_color(CC_LIGH_BLUE);
    draw_ui_button(&s_button_position, s_saved_position.x, s_saved_position.y, 200, 360);

    s_timer_animation += t_delta_time();
    if (s_timer_animation >= .1f) {

        s_index_animation ++;
        s_timer_animation = 0;

        if (s_index_animation == 13)
         s_index_animation = 0;

        s_sprite_spritesheet_idle.texture_slice.x = (s_index_animation % 4) * 200;
        s_sprite_spritesheet_idle.texture_slice.y = (s_index_animation / 4) * 360;
    }

    draw_ui_button(&s_button_quit, 16, 16, 96, 32);
    t_vec2 size_text_quit = measure_text_size_ttf("Quit", &s_font_ui_m);
    draw_text_ttf("Quit", &s_font_ui_m, (t_vec2) { 16 + (96 - size_text_quit.x) / 2, 16 + (32 + size_text_quit.y) / 2}, CC_BLACK, 0);

    draw_ui_button(&s_button_save, 16, 56, 96, 32);
    t_vec2 size_text_save = measure_text_size_ttf("Save", &s_font_ui_m);
    draw_text_ttf("Save", &s_font_ui_m, (t_vec2) { 16 + (96 - size_text_save.x) / 2, 56 + (32 + size_text_save.y) / 2}, CC_BLACK, 0);


    if (s_prompt_save) {
        t_color modal_background_color = CC_BLACK;
        modal_background_color.a /= 2;

        set_ui_raycast_block_r(RECT_ZERO);

        draw_rect(0, 0, t_window_size().x, t_window_size().y, modal_background_color);
        draw_sprite(&s_sprite_button, (t_window_size().x - 256) / 2, (t_window_size().y - 128) / 2, 256, 128, CC_LIGHT_RED);

        t_vec2 size_text = measure_text_size_ttf("Exit without saving?", &s_font_ui_m);
        draw_text_ttf("Exit without saving?", &s_font_ui_m, (t_vec2){(t_window_size().x - size_text.x) / 2, (t_window_size().y + size_text.y) / 2 - size_text.y}, CC_BLACK, 0);

        t_vec2 size_text_no = measure_text_size_ttf("No", &s_font_ui_m);
        draw_ui_button(&s_button_quit_no, 
            (t_window_size().x - size_text.x) / 2 + 32, (t_window_size().y - 128) / 2 + 128 - 48, size_text_no.x, size_text_no.y);

        t_vec2 size_text_yes = measure_text_size_ttf("Yes", &s_font_ui_m);
        draw_ui_button(&s_button_quit_yes,
            (t_window_size().x - size_text.x) / 2 + size_text.x - size_text_yes.x - 32, (t_window_size().y - 128) / 2 + 128 - 48, size_text_yes.x, size_text_yes.y);

        set_ui_raycast_block(0, 0, t_window_size().x, t_window_size().y);
    }

    if (!s_prompt_save)
        set_ui_raycast_block_r(RECT_ZERO);

    clear_ui();
}