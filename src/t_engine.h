#pragma once

#include "stdbool.h"

int t_begin(int window_width, int window_height, const char* title);
bool t_loop();
void t_loop_end();
void t_end();

void t_play_audio(const char* path);
void t_set_cursor(const char* path);