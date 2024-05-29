#include <stdlib.h>
#include <stdio.h>

#include "./engine/tailored.h"

#include "screens.h"
#include "game.h"

#include "./engine/extern/stb_truetype.h"

//#include <stdatomic.h>

static bool s_loading = false;
//
//#ifdef _WIN32
//    #include <threads.h>
//    static thrd_t s_thread;
//        #define CREATE_THREAD(thread, func) thrd_create(thread, func, NULL)
//        #define JOIN_THREAD(thread) thrd_join(thread, NULL)
//#else
//    #include <pthread.h>
//    static pthread_t s_thread;
//        #define CREATE_THREAD(thread, func) pthread_create(thread, NULL, func, NULL)
//        #define JOIN_THREAD(thread) pthread_join(thread, NULL)
//#endif
//static atomic_int s_loading_progress = 0;
//static atomic_bool s_loading_finished = false;

static float s_loading_buffer = 0;


static t_screen m_current_screen = NONE;
static bool s_should_change_screen = false;
static t_screen s_should_change_screen_to;

static t_sprite s_sprite_loading_bar;

static t_font s_font;
static float s_fps_update = 0;
static int s_current_fps;

char* g_save_file;

void set_loading_progress(float progress) {
    //atomic_store_explicit(&s_loading_progress, (int)(progress * 10), memory_order_relaxed);
}
void set_loading_finished() {
    //atomic_store_explicit(&s_loading_finished, true, memory_order_relaxed);
}

void set_screen(t_screen screen) {
    s_should_change_screen = true;
    s_should_change_screen_to = screen;
}

int main() {

    t_result result = t_begin(640, 360, "Tailored");

    if (result != T_SUCCESS) {
        exit(EXIT_FAILURE);
    }

    t_set_cursor("./res/textures/pointer_b.png", 8, 6);
    t_load_texture_data_s(&s_sprite_loading_bar, "./res/textures/loading_bar.png");
    t_init_sprite(&s_sprite_loading_bar);
    s_font = load_ttf_font("./res/fonts/Roboto-Regular.ttf", 18);

    set_screen(SPLASH);

    while(t_loop()) {

        if (s_should_change_screen)
        {
            s_should_change_screen = false;

            switch (m_current_screen)
            {
                case SPLASH: unload_splash_screen(); break;
                case TITLE: unload_title_screen(); break;
                case GAME: unload_game_screen(); break;
                case SETUP: unload_setup_screen(); break;
                case NONE: break;
            }

            int (*load_screen)(void*) = NULL;
            switch (s_should_change_screen_to)
            {
                case SPLASH: load_screen = &load_splash_screen; break;
                case TITLE: load_screen = &load_title_screen; break;
                case GAME: load_screen = &load_game_screen; break;
                case SETUP: load_screen = &load_setup_screen; break;
                case NONE: break;
            }

            if (load_screen != NULL) {
                load_screen(NULL);

                s_loading = true;
                //s_loading_finished = false;
                //s_loading_progress = 0;
                //s_loading_buffer = 0;
                //int result = CREATE_THREAD(&s_thread, load_screen);
                //if (result != 0) {
                //    printf("thrd_create failed, error: %d\n", result);
                //} else {
                //    printf("Loading screen started.\n");
                //}
            }

            m_current_screen = s_should_change_screen_to;
        }

        if (s_loading) {
            switch (m_current_screen)
            {
            case SPLASH: init_splash_screen(); break;
            case TITLE: init_title_screen(); break;
            case GAME: init_game_screen(); break;
            case SETUP: init_setup_screen(); break;
            case NONE: break;
            }
            s_loading = false;
            //s_loading_buffer += t_delta_time();
           /* if (atomic_load_explicit(&s_loading_finished, memory_order_relaxed)) {
                s_loading = false;

                int result = JOIN_THREAD(s_thread);
                if (result != 0) {
                    printf("thrd_join failed, error: %d\n", result);
                } else {
                    printf("Loading screen finished.\n");
                    printf("Initializing screen.\n");
                    switch (m_current_screen)
                    {
                        case SPLASH: init_splash_screen(); break;
                        case TITLE: init_title_screen(); break;
                        case GAME: init_game_screen(); break;
                        case SETUP: init_setup_screen(); break;
                        case NONE: break;
                    }
                    printf("Initializing screen finished.\n");
                }
            }
            else if (s_loading_buffer > .15f){
                t_clear_color(CC_BLACK);
                t_draw_sprite(&s_sprite_loading_bar,
                    (t_window_size().x - s_sprite_loading_bar.texture_data.width) / 2,
                    (t_window_size().y - s_sprite_loading_bar.texture_data.height) / 2,
                    s_sprite_loading_bar.texture_data.width,
                    s_sprite_loading_bar.texture_data.height, CC_RED);

                draw_rect(
                    (t_window_size().x - s_sprite_loading_bar.texture_data.width) / 2,
                    t_window_size().y / 2 - s_sprite_loading_bar.texture_data.height / 4,
                    s_sprite_loading_bar.texture_data.width * ((float)atomic_load_explicit(&s_loading_progress, memory_order_relaxed) / 10),
                    s_sprite_loading_bar.texture_data.height / 2, CC_RED);
            }*/
        }

        if (!s_loading) {
            switch (m_current_screen)
            {
                case SPLASH: draw_splash_screen(); break;
                case TITLE: draw_title_screen(); break;
                case GAME: draw_game_screen(); break;
                case SETUP: draw_setup_screen(); break;
                case NONE: break;
            }
        }

        s_fps_update += t_delta_time();
        if (s_fps_update >= 1.0f) { 
            s_current_fps = (int)(1.0f / t_delta_time());
            s_fps_update = 0;
        }
        
        // char* text_fps[20];
        // sprintf(text_fps, "%d fps", s_current_fps);
        // t_draw_text(text_fps, &s_font, (t_vec2) { 12, t_window_size().y - 12 }, WHITE, 0);

        t_draw_scene();
        t_loop_end();
    }

    t_end();
    return 0;
}
