#include <stdlib.h>
#include <stdio.h>

#include "./engine/tailored.h"
#include "./engine/t_sprite.h"
#include "./engine/t_font.h"
#include "./engine/t_ui.h"

#include "screens.h"

#include "./engine/extern/stb_truetype.h"

t_screen m_current_screen = TITLE;

bool m_should_change_screen = false;
t_screen m_should_change_screen_to;

void on_free_button_released() {
}

int main() {

    t_result result = t_begin(640, 360, "Imps & Fairies");

    if (result != t_success) {
        exit(EXIT_FAILURE);
    }

    t_set_cursor("./res/textures/pointer_b.png", 8, 6);
    load_title_screen();

    while(t_loop()) {

        switch (m_current_screen)
        {
            case SPLASH: update_splash_screen(); break;
            case TITLE: update_title_screen(); break;
            case GAME: update_game_screen(); break;
        }

        if (m_should_change_screen)
        {
            m_should_change_screen = false;
            switch (m_current_screen)
            {
                case SPLASH: unload_splash_screen(); break;
                case TITLE: unload_title_screen(); break;
                case GAME: unload_game_screen(); break;
            }

            switch (m_should_change_screen_to)
            {
                case SPLASH: load_splash_screen(); break;
                case TITLE: load_title_screen(); break;
                case GAME: load_game_screen(); break;
            }

            m_current_screen = m_should_change_screen_to;
        }

        switch (m_current_screen)
        {
            case SPLASH: draw_splash_screen(); break;
            case TITLE: draw_title_screen(); break;
            case GAME: draw_game_screen(); break;
        }

        t_draw_scene();
        t_loop_end();
    }

    t_end();
    return 0;
}
