#include "../../engine/t_ui.h"

void load_section_saves();
void draw_section_saves(const float p_offset_x, const float p_offset_y);
void unload_section_saves();
void set_on_save_file_loaded(void (*on_loaded)(struct t_ui_button*));

void load_section_settings();
void draw_section_settings(const float p_offset_x, const float p_offset_y);
void unload_section_settings();

void load_section_about();
void draw_section_about(const float p_offset_x, const float p_offset_y);
void unload_section_about();