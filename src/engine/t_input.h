#pragma once

#include <stdbool.h>

//mouse
bool is_mouse_button_pressed(int button);
bool is_mouse_button_down(int button);
bool is_mouse_button_released(int button);

t_vec2 mouse_position();

//keyboard
bool is_key_pressed(int key);
bool is_key_down(int key);
bool is_key_released(int key);

#define MOUSE_BUTTON_LEFT 0
#define MOUSE_BUTTON_RIGHT 1