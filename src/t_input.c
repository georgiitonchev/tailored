#include "t_core.h";
#include "t_input.h"

extern t_global_state global_state;

bool is_mouse_button_pressed(int button) {
    return global_state.input_state.mouse_state.buttons[button].is_pressed;
}

bool is_mouse_button_down(int button) {
    return global_state.input_state.mouse_state.buttons[button].is_down;
}

bool is_mouse_button_released(int button) {
    return global_state.input_state.mouse_state.buttons[button].is_released;
}

//keyboard
bool is_key_pressed(int key){ return false;}
bool is_key_down(int key) {return false;}
bool is_key_released(int key) {return false;}