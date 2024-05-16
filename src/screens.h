#pragma once

typedef enum t_screen { NONE, SPLASH, TITLE, GAME } t_screen;

void set_screen(t_screen screen);

int load_splash_screen(void* args);
void unload_splash_screen();
void init_splash_screen();
void draw_splash_screen();

int load_title_screen(void* args);
void unload_title_screen();
void init_title_screen();
void draw_title_screen();

int load_game_screen(void* args);
void unload_game_screen();
void init_game_screen();
void draw_game_screen();

void set_loading_progress(float progress);
void set_loading_finished();