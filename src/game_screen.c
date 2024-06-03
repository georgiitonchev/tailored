#include "screens.h"

#include "./engine/tailored.h"
#include "game.h"

#include "stdio.h"
#include "stdlib.h"
#include "stdbool.h"

#include "cJSON.h"

// EXTERNS
extern char* g_save_file;

// FONTS
static t_font s_font_ui_s;
static t_font s_font_ui_m;

// SPRITES
static t_sprite s_sprite_button;
static t_sprite s_sprite_spritesheet_idle;

// UI
static t_ui_button s_button_save;
static t_ui_button s_button_reset;
static t_ui_button s_button_quit;

static t_ui_button s_button_quit_yes;
static t_ui_button s_button_quit_no;

static t_ui_button s_button_position;

// STATE
static t_vec2 s_saved_position;
static cJSON* s_json_data;

static bool s_has_saved = true;
static bool s_prompt_save = false;

static bool s_drawing_categories = true;
static bool s_drawing_options = false;

static float s_ease_out_timer_left_side = 0;
static float s_ease_out_left_side = false;
static float s_ease_in_left_side = false;

static float s_ease_out_timer_right_side = 0;
static float s_ease_out_right_side = false;
static float s_ease_in_right_side = false;

static float s_left_side_offset_x = 0;
static float s_right_side_offset_x = 0;

static void s_on_button_position_pressed(t_ui_button* button) {
    s_drawing_categories = !s_drawing_categories;
    s_drawing_options = !s_drawing_options;
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

static void s_on_button_reset_clicked() {

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

static const char* s_categories[] = {"Horns", "Hair", "Body", "Eyes", "Nails", "Wings", "Ears", "Mouth", "Tail"};
static void s_init_categories() { 

}

void init_game_screen() {

    s_ease_in_left_side = true;
    s_ease_in_right_side = true;

    s_font_ui_s = t_load_ttf_font("./res/fonts/Eczar-Regular.ttf", 32);
    s_font_ui_m = t_load_ttf_font("./res/fonts/Eczar-Regular.ttf", 34);

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

    s_button_reset = create_ui_button(&s_sprite_button);
    s_button_reset.color_default = CC_LIGHT_RED;
    s_button_reset.color_disabled = CC_RED;
    s_button_reset.on_released = s_on_button_reset_clicked;

    s_button_quit_yes = create_ui_button_t(&s_font_ui_m, "Yes");
    s_button_quit_yes.on_released = s_on_button_quit_yes_clicked;

    s_button_quit_no = create_ui_button_t(&s_font_ui_m, "No");
    s_button_quit_no.on_released = s_on_button_quit_no_clicked;

    t_init_sprite(&s_sprite_spritesheet_idle);
    s_sprite_spritesheet_idle.texture_slice = (t_vec4) { 0, 0, 200, 360};

    s_button_position = create_ui_button(&s_sprite_spritesheet_idle);
    s_button_position.color_default = WHITE;
    s_button_position.on_released = s_on_button_position_pressed;
}

void unload_game_screen() {
    t_deinit_sprite(&s_sprite_button);
    t_delete_ttf_font(&s_font_ui_m);

    cJSON_Delete(s_json_data);
}

static void s_draw_categories() {

    unsigned int category_index = 0;

    for (int x = 0; x < 3; x++) {
        for (int y = 0; y < 3; y++) {

            const float pos_x = 376 + 16 + x * (64 + 12) + s_right_side_offset_x;
            const float pos_y = 16 + 16 + y * (64 + 45);

            t_color color = CC_BLACK;
            color.a /= 2;

            draw_rect(pos_x, pos_y, 64, 64, color);

            t_vec2 text_size = t_measure_text_size(s_categories[category_index], &s_font_ui_s);
            t_draw_text(s_categories[category_index], &s_font_ui_s, pos_x + (64 - text_size.x) / 2,
                  pos_y + 64 + text_size.y + 6 , CC_BLACK, 0);

            category_index++;
        }
    }
}

static void s_draw_options() {

    t_vec2 size_category_text = t_measure_text_size("Horns", &s_font_ui_m);
    t_draw_text("Horns", &s_font_ui_m, 376 + (248 - size_category_text.x) / 2 + s_right_side_offset_x, 16 + size_category_text.y + 12, CC_BLACK, 0);

    t_vec2 size_style_text = t_measure_text_size("Style", &s_font_ui_s);
    t_draw_text("Style", &s_font_ui_s, 376 + (248 - size_style_text.x) / 2 + s_right_side_offset_x, 52 + size_style_text.y, CC_BLACK, 0);

    for (int x = 0; x < 4; x++) {
        for (int y = 0; y < 3; y++) {

            const float pos_x = 376 + 42 + x * (32 + 12) + s_right_side_offset_x;
            const float pos_y = 78 + y * (32 + 12);

            t_color color = CC_BLACK;
            color.a /= 2;
            draw_rect(pos_x, pos_y, 32, 32, color);
        }
    }

    t_vec2 size_color_text = t_measure_text_size("Color", &s_font_ui_s);
    t_draw_text("Color", &s_font_ui_s, 376 + (248 - size_color_text.x) / 2 + s_right_side_offset_x, 216 + size_color_text.y , CC_BLACK, 0);

    for (int x = 0; x < 11; x++) {
        for (int y = 0; y < 4; y++) {

            const float pos_x = 376 + 42 + x * (12 + 3) + s_right_side_offset_x;
            const float pos_y = 239 + y * (12 + 3);

            t_color color = CC_BLACK;
            color.a /= 2;
            draw_rect(pos_x, pos_y, 12, 12, color);
        }
    }
}

static float s_timer_animation = 0;
static int s_index_animation = 1;

void draw_game_screen() {
    t_clear_color(CC_BLACK);
    draw_ui_button(&s_button_position, 144, 0, 200, 360);

    s_timer_animation += t_delta_time();
    if (s_timer_animation >= .1f) {

        s_index_animation ++;
        s_timer_animation = 0;

        if (s_index_animation == 13)
         s_index_animation = 0;

        s_sprite_spritesheet_idle.texture_slice.x = (s_index_animation % 4) * 200;
        s_sprite_spritesheet_idle.texture_slice.y = (s_index_animation / 4) * 360;
    }

    if (s_ease_in_left_side) {

        float progress = t_ease_out_quint(&s_ease_out_timer_left_side, &s_left_side_offset_x, -256, 0, .5f);

        if (progress >= 1) {
            s_ease_out_timer_left_side = 0;
            s_ease_in_left_side = false;
        }
    }

    if (s_ease_out_left_side) {

        float progress = t_ease_out_quint(&s_ease_out_timer_left_side, &s_left_side_offset_x, 0, -256, .5f);

        if (progress >= 1) {
            s_ease_out_timer_left_side = 0;
            s_ease_out_left_side = false;
        }
    }

    draw_ui_button(&s_button_save, 16 + s_left_side_offset_x, 16, 96, 32);
    t_vec2 size_text_save = t_measure_text_size("Save", &s_font_ui_m);
    t_draw_text("Save", &s_font_ui_m, 16 + (96 - size_text_save.x) / 2 + s_left_side_offset_x, 16 + (32 + size_text_save.y) / 2, CC_BLACK, 0);

    draw_ui_button(&s_button_reset, 16 + s_left_side_offset_x, 56, 96, 32);
    t_vec2 size_text_reset = t_measure_text_size("Reset", &s_font_ui_m);
    t_draw_text("Reset", &s_font_ui_m, 16 + (96 - size_text_reset.x) / 2 + s_left_side_offset_x,  56 + (32 + size_text_reset.y) / 2, CC_BLACK, 0);

    draw_ui_button(&s_button_quit, 16 + s_left_side_offset_x, t_window_size().y - 32 - 16, 96, 32);
    t_vec2 size_text_quit = t_measure_text_size("Back", &s_font_ui_m);
    t_draw_text("Back", &s_font_ui_m, 16 + (96 - size_text_quit.x) / 2 + s_left_side_offset_x, t_window_size().y - 32 - 16 + (32 + size_text_quit.y) / 2, CC_BLACK, 0);


    if (s_ease_in_right_side) {

        float progress = t_ease_out_quint(&s_ease_out_timer_right_side, &s_right_side_offset_x, 256, 0, .5f);

        if (progress >= 1) {
            s_ease_out_timer_right_side = 0;
            s_ease_in_right_side = false;
        }
    }

    if (s_ease_out_right_side) {

        float progress = t_ease_out_quint(&s_ease_out_timer_right_side, &s_right_side_offset_x, 0, 256, .5f);

        if (progress >= 1) {
            s_ease_out_timer_right_side = 0;
            s_ease_out_right_side = false;
        }
    }

    t_draw_sprite(&s_sprite_button, 376 + s_right_side_offset_x, 16, 248, 328, CC_LIGHT_RED);

    // Categories
    if (s_drawing_categories)
        s_draw_categories();

    // Options
    if (s_drawing_options)
        s_draw_options();

    if (s_prompt_save) {
        t_color modal_background_color = CC_BLACK;
        modal_background_color.a /= 2;

        set_ui_raycast_block_r(RECT_ZERO);

        draw_rect(0, 0, t_window_size().x, t_window_size().y, modal_background_color);
        t_draw_sprite(&s_sprite_button, (t_window_size().x - 256) / 2, (t_window_size().y - 128) / 2, 256, 128, CC_LIGHT_RED);

        t_vec2 size_text = t_measure_text_size("Exit without saving?", &s_font_ui_m);
        t_draw_text("Exit without saving?", &s_font_ui_m, (t_window_size().x - size_text.x) / 2, (t_window_size().y + size_text.y) / 2 - size_text.y, CC_BLACK, 0);

        t_vec2 size_text_no = t_measure_text_size("No", &s_font_ui_m);
        draw_ui_button(&s_button_quit_no,
            (t_window_size().x - size_text.x) / 2 + 32, (t_window_size().y - 128) / 2 + 128 - 48, size_text_no.x, size_text_no.y);

        t_vec2 size_text_yes = t_measure_text_size("Yes", &s_font_ui_m);
        draw_ui_button(&s_button_quit_yes,
            (t_window_size().x - size_text.x) / 2 + size_text.x - size_text_yes.x - 32, (t_window_size().y - 128) / 2 + 128 - 48, size_text_yes.x, size_text_yes.y);

        set_ui_raycast_block(0, 0, t_window_size().x, t_window_size().y);
    }

    if (!s_prompt_save)
        set_ui_raycast_block_r(RECT_ZERO);

    clear_ui();
}
