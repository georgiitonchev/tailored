#pragma once

typedef enum t_screen { SPLASH, TITLE, GAME } t_screen;

void load_splash_screen();
void unload_splash_screen();
void update_splash_screen();
void draw_splash_screen();

void load_title_screen();
void unload_title_screen();
void update_title_screen();
void draw_title_screen();

void load_game_screen();
void unload_game_screen();
void update_game_screen();
void draw_game_screen();