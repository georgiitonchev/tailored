#include "screens.h"
#include "./engine/tailored.h"

#include "game.h"

extern bool m_should_change_screen;
extern t_screen m_should_change_screen_to;

static float s_splash_timer = 0;

void load_splash_screen() {

}

void unload_splash_screen() {

}

void update_splash_screen() {

}

void draw_splash_screen() {
     t_clear_color(CC_BLACK);

     if (s_splash_timer >= 1) {
          m_should_change_screen = true;
          m_should_change_screen_to = TITLE;
     }

     s_splash_timer += t_delta_time();
}