#include <stdbool.h>

#include "t_core.h"
#include "t_sprite.h"

typedef struct t_ui_el {

} t_ui_el;

typedef struct t_ui_btn {

    t_rect rect;
    t_sprite* sprite;

    t_color color_default;
    t_color color_mouseover;
    t_color color_clicked;
    t_color color_disabled;

    void (*on_released)(struct t_ui_btn*);
    void (*on_mouse_enter)();
    void (*on_mouse_exit)();
    void (*on_pressed)();

    bool mouse_entered;
    bool was_clicked;
    bool is_selected;

} t_ui_btn;

void draw_ui_button(t_ui_btn* button);