#include "screen_title.h"

#include "../../engine/tailored.h"
#include "../../../dep/include/glad/glad.h"
#include "../../game.h"
#include "../../cJSON.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// EXTERN
extern char* g_save_file;

static const char* save_files_data_path = "saves/save_files_data.tsfs";

typedef struct save_file_ui_data {
    const char* save_file_name;

    float fade_in_y;
    float fade_in_timer;

    unsigned int index;

} save_file_ui_data;

// FONTS
static t_font s_font_ui_s;
static t_font s_font_ui_xs;
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
static float s_scroll_area_width;
static t_ui_button* s_selected_save = NULL;
static t_rect s_rect_slider;
static void (*s_on_loaded)(struct t_ui_button*);

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

static void s_on_save_file_loaded(t_ui_button* button) {
    UNUSED(button);

    save_file_ui_data* sfui_data = (save_file_ui_data*)s_selected_save->user_data;

    g_save_file = sfui_data->save_file_name;
    s_on_loaded(s_selected_save);
}

static void s_on_button_delete_clicked(t_ui_button* button) { 
    UNUSED(button);

    long file_size;
    const char* save_files_data = t_read_file(save_files_data_path, &file_size);

    if (save_files_data != NULL) {
        
        cJSON* json_data = cJSON_Parse(save_files_data);
        cJSON* json_files_array = cJSON_GetObjectItem(json_data, "save_files");

        save_file_ui_data* sfui_data = (save_file_ui_data*)s_selected_save->user_data;

        cJSON* json_array_item = cJSON_GetArrayItem(json_files_array, sfui_data->index);

        // remove(json_array_item->valuestring);
        remove_from_list(s_list_saves, sfui_data->index);

        cJSON_DeleteItemFromArray(json_files_array, sfui_data->index);
        const char* json_string = cJSON_Print(json_data);

        t_write_file(save_files_data_path, json_string);
    }
}

static void s_on_button_new_clicked() {

    time_t current_time;
    struct tm* time_info;

    char* time_string = malloc(20 * sizeof(char));
    char file_name_string[20];

    time(&current_time);
    time_info = localtime(&current_time);

    strftime(time_string, 20, "%H:%M:%S", time_info);
    strftime(file_name_string, 20, "%H-%M-%S", time_info);

    cJSON* json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "created_at", time_string);

    const char* json_string = cJSON_Print(json);
    char* file_name = malloc(100 * sizeof(char));

    sprintf(file_name, "saves/%s.tsf", file_name_string);
    
    t_result result = t_write_file(file_name, json_string);
    if (result == T_SUCCESS) { 
        
        t_ui_button* character_button = (t_ui_button*)malloc(sizeof(t_ui_button));
        *character_button = create_ui_button(&s_sprite_button);

        character_button->color_default = CC_BLACK;
        character_button->color_mouseover = CC_DARK_RED;
        character_button->color_clicked = CC_RED;

        save_file_ui_data* sfui_data = (save_file_ui_data*)malloc(sizeof(save_file_ui_data));
        sfui_data->save_file_name = file_name;
        sfui_data->fade_in_y = 0;
        sfui_data->fade_in_timer = 0;
        sfui_data->index = s_list_saves->size;

        character_button->user_data = sfui_data;
        character_button->on_released = s_on_save_file_clicked;

        add_to_list(s_list_saves, character_button);

        s_scroll_area_width = s_list_saves->size * (128 + 16) - 16;

        long file_size;
        const char* save_files_data = t_read_file(save_files_data_path, &file_size);

        if (save_files_data != NULL) {
            cJSON* json_data = cJSON_Parse(save_files_data);
            cJSON* json_files_array = cJSON_GetObjectItem(json_data, "save_files");
            cJSON* json_file_name = cJSON_CreateString(file_name);

            cJSON_AddItemToArray(json_files_array, json_file_name);
            const char* json_string = cJSON_Print(json_data);

            t_write_file(save_files_data_path, json_string);
            printf("Save files data updated.\n");
        }
    }
}

static void s_on_slider_knob_button_pressed() { 

    float value = (mouse_position().x - (s_rect_slider.x + s_button_slider_knob.sprite->texture_data.width / 2)) 
        / (s_rect_slider.width - s_button_slider_knob.sprite->texture_data.width);

    if (value < 0)
        value = 0;
    else if (value > 1)
        value = 1;

    s_scroll_area_offset = value;
}

void load_section_saves() {
    // Load textures
    t_load_texture_data_s(&s_sprite_button, "./res/textures/panel-transparent-center-030.png");
    t_load_texture_data_s(&s_sprite_button_selected, "./res/textures/panel-transparent-center-029.png");
    t_load_texture_data_s(&s_sprite_section_background, "./res/textures/panel-transparent-center-030.png");
    t_load_texture_data_s(&s_sprite_slider_background, "./res/textures/slider_background.png");
    t_load_texture_data_s(&s_sprite_small_knob, "./res/textures/slider_knob_small.png");
    t_load_texture_data_s(&s_sprite_big_knob, "./res/textures/slider_knob_big.png");
}

void init_section_saves() {

    t_init_sprite(&s_sprite_button);
    s_sprite_button.slice_borders = (t_vec4){ 16, 16, 16, 16 };

    t_init_sprite(&s_sprite_button_selected);
    s_sprite_button_selected.slice_borders = (t_vec4){ 16, 16, 16, 16 };

    t_init_sprite(&s_sprite_section_background);
    s_sprite_section_background.slice_borders = (t_vec4){ 16, 16, 16, 16 };

    t_init_sprite(&s_sprite_slider_background);
    t_init_sprite(&s_sprite_small_knob);
    t_init_sprite(&s_sprite_big_knob);
    
    s_font_ui_s = load_ttf_font("./res/fonts/Eczar-Regular.ttf", 32);
    s_font_ui_xs = load_ttf_font("./res/fonts/Eczar-Regular.ttf", 27);
    s_font_ui_m = load_ttf_font("./res/fonts/Eczar-Regular.ttf", 37);

    s_button_slider_knob = create_ui_button(&s_sprite_big_knob);
    s_button_slider_knob.color_default = CC_BLACK;
    s_button_slider_knob.color_mouseover = CC_DARK_RED;
    s_button_slider_knob.color_clicked = CC_DARK_RED;
    s_button_slider_knob.on_pressed = s_on_slider_knob_button_pressed;

    s_button_new = create_ui_button(&s_sprite_button);
    s_button_new.color_default = CC_BLACK;
    s_button_new.color_mouseover = CC_DARK_RED;
    s_button_new.color_clicked = CC_RED;
    s_button_new.color_disabled = CC_DARK_RED;
    s_button_new.on_released = s_on_button_new_clicked;
    // s_button_new.on_mouse_enter = on_button_mouse_enter;

    s_button_delete = create_ui_button(&s_sprite_button);
    s_button_delete.color_default = CC_BLACK;
    s_button_delete.color_mouseover = CC_DARK_RED;
    s_button_delete.color_clicked = CC_RED;
    s_button_delete.color_disabled = CC_DARK_RED;
    s_button_delete.is_disabled = true;
    s_button_delete.on_released = s_on_button_delete_clicked;
    // s_button_delete.on_mouse_enter = on_button_mouse_enter;

    s_button_load = create_ui_button(&s_sprite_button);
    s_button_load.color_default = CC_BLACK;
    s_button_load.color_mouseover = CC_DARK_RED;
    s_button_load.color_clicked = CC_RED;
    s_button_load.color_disabled = CC_DARK_RED;
    s_button_load.is_disabled = true;
    // s_button_load.on_mouse_enter = on_button_mouse_enter;
    s_button_load.on_released = s_on_save_file_loaded;

    s_list_saves = create_list(sizeof(t_ui_button));

    long file_size;
    const char* save_files_data = t_read_file(save_files_data_path, &file_size);

    if (save_files_data == NULL) {

       cJSON* json_data = cJSON_CreateObject();
       cJSON_AddArrayToObject(json_data, "save_files");

       const char* json_string = cJSON_Print(json_data);
       t_write_file(save_files_data_path, json_string);

    } else {

        cJSON* json_data = cJSON_Parse(save_files_data);
        cJSON* json_files_array = cJSON_GetObjectItem(json_data, "save_files");
        int json_files_array_size = cJSON_GetArraySize(json_files_array);

        for (int i = 0; i < json_files_array_size; i++) {

            cJSON* json_array_item = cJSON_GetArrayItem(json_files_array, i);

            long save_file_data_size;
            const char* save_file_data = t_read_file(json_array_item->valuestring, &save_file_data_size);

            cJSON* sf_json_data = cJSON_Parse(save_file_data);
            cJSON* sf_created_at = cJSON_GetObjectItem(sf_json_data, "created_at");

            t_ui_button* character_button = (t_ui_button*)malloc(sizeof(t_ui_button));
            *character_button = create_ui_button(&s_sprite_button);

            character_button->color_default = CC_BLACK;
            character_button->color_mouseover = CC_DARK_RED;
            character_button->color_clicked = CC_RED;

            save_file_ui_data* sfui_data = (save_file_ui_data*)malloc(sizeof(save_file_ui_data));
            sfui_data->save_file_name = json_array_item->valuestring;
            sfui_data->fade_in_y = 0;
            sfui_data->fade_in_timer = 0;
            sfui_data->index = i;

            character_button->user_data = sfui_data;
            character_button->on_released = s_on_save_file_clicked;

            add_to_list(s_list_saves, character_button);
        }

        s_scroll_area_width = s_list_saves->size * (128 + 16) - 16;
    }
}

void draw_section_saves(const float p_offset_x, const float p_offset_y) {

    t_draw_sprite(&s_sprite_section_background, 256 + p_offset_x, 16 + p_offset_y, 368, 328, CC_LIGHT_RED);

    // CHARACTER ELEMENTS
    t_begin_scissor(256 + 16 + p_offset_x, 16 + p_offset_y, 368 - 32, 223 + 16);
    for (unsigned int i = 0; i < s_list_saves->size; i ++) {

        t_ui_button* character_button = (t_ui_button*)element_at_list(s_list_saves, i);
        save_file_ui_data* sfui_data = (save_file_ui_data*)character_button->user_data;

        float offset_x = (s_scroll_area_width - (s_rect_slider.width)) * s_scroll_area_offset;

        if (sfui_data->fade_in_timer <= 1)
            t_ease_out_quint(&sfui_data->fade_in_timer, &sfui_data->fade_in_y, 223, 0, .25f);

        draw_ui_button(character_button, (272 + i * 128 + i * 16) - offset_x + p_offset_x, 32 + p_offset_y + sfui_data->fade_in_y, 128, 223);
        draw_rect((272 + i * 128 + i * 16) - offset_x + 4 + p_offset_x, 32 + p_offset_y + 223 - 48 + sfui_data->fade_in_y, 120, 32, (t_color) {12, 12, 12, 100});

        t_vec2 text_size = measure_text_size_ttf(sfui_data->save_file_name, &s_font_ui_xs);
        draw_text_ttf(sfui_data->save_file_name, &s_font_ui_xs, (t_vec2){ (272 + i * 128 + i * 16) - offset_x + (128 - text_size.x) / 2 + p_offset_x, 32 + p_offset_y + 223 - 32 + sfui_data->fade_in_y + text_size.y / 2}, CC_RED, 0);
    }
    t_end_scissor();

    s_rect_slider = (t_rect) { 272 + p_offset_x, 328 - 64 + p_offset_y, 336, s_sprite_slider_background.texture_data.height };

    // BIG KNOB
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glClear(GL_STENCIL_BUFFER_BIT);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);

    if (s_list_saves->size > 0)
        draw_ui_button(&s_button_slider_knob, 272 + p_offset_x + s_scroll_area_offset * (s_rect_slider.width - s_button_slider_knob.sprite->texture_data.width), 328 - 66 + p_offset_y, s_sprite_big_knob.texture_data.width, s_sprite_big_knob.texture_data.height);

    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    if (s_list_saves->size > 0 && !s_button_slider_knob.is_mouse_over && is_point_in_rect(mouse_position(), s_rect_slider)) {

        if (is_mouse_button_pressed(MOUSE_BUTTON_LEFT)) {
            s_button_slider_knob.was_clicked = true;
            s_button_slider_knob.mouse_clicked_at = (t_vec2) { s_button_slider_knob.sprite->texture_data.width / 2, s_button_slider_knob.sprite->texture_data.height / 2};
        }

        t_rect rect_small_knob = (t_rect) { mouse_position().x - s_sprite_small_knob.texture_data.width / 2 + p_offset_x, s_rect_slider.y + p_offset_y, s_sprite_small_knob.texture_data.width, s_sprite_small_knob.texture_data.height };

        //limits
        if (rect_small_knob.x < s_rect_slider.x)
            rect_small_knob.x = s_rect_slider.x;
        else if (rect_small_knob.x > s_rect_slider.x + s_rect_slider.width - rect_small_knob.width)
            rect_small_knob.x = s_rect_slider.x + s_rect_slider.width - rect_small_knob.width;

        t_draw_sprite_r(&s_sprite_small_knob, rect_small_knob, CC_BLACK);
    }

    glStencilMask(0x00);
    // SLIDER
    t_draw_sprite_r(&s_sprite_slider_background, s_rect_slider, CC_BLACK);

    glStencilMask(0xFF);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);

    if (s_list_saves->size == 0) {
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
    destroy_list(s_list_saves);

    delete_ttf_font(&s_font_ui_xs);
    delete_ttf_font(&s_font_ui_s);
    delete_ttf_font(&s_font_ui_m);

    t_deinit_sprite(&s_sprite_button);
    t_deinit_sprite(&s_sprite_button_selected);
    t_deinit_sprite(&s_sprite_section_background);
    t_deinit_sprite(&s_sprite_slider_background);
    t_deinit_sprite(&s_sprite_small_knob);
    t_deinit_sprite(&s_sprite_big_knob);

    s_selected_save = NULL;
    s_scroll_area_offset = 0;
}

void set_on_save_file_loaded(void (*on_loaded)(struct t_ui_button*)) {
    s_on_loaded = on_loaded;
}