#include "screens.h"
#include "./engine/tailored.h"
#include "./engine/t_easings.h"

#include "game.h"

extern bool m_should_change_screen;
extern t_screen m_should_change_screen_to;

static float s_splash_timer = 0;

static t_sprite s_sprite_splash_logo;
static t_vec2 s_window_size;

static float s_timer_splash_fade = 0;
static float s_splash_fade = 0;

void load_splash_screen() {
     create_sprite("./res/textures/splash_logo.png", &s_sprite_splash_logo);

     s_window_size = t_window_size();
}

void unload_splash_screen() {

}

void update_splash_screen() {

}

void draw_splash_screen() {
     t_clear_color(CC_BLACK);

     t_color logo_color = CC_RED;
     logo_color.a -= CC_RED.a * s_splash_fade;

     if (s_splash_timer > 1)
     t_ease_in_back(&s_timer_splash_fade, &s_splash_fade, 0, 1, 2.5f);

     draw_sprite(&s_sprite_splash_logo, (s_window_size.x - s_sprite_splash_logo.texture.size.x) / 2, (s_window_size.y - s_sprite_splash_logo.texture.size.y) / 2, 
          s_sprite_splash_logo.texture.size.x, s_sprite_splash_logo.texture.size.y, logo_color);

     if (s_splash_timer >= 4) {
          m_should_change_screen = true;
          m_should_change_screen_to = TITLE;
     }

     s_splash_timer += t_delta_time();
}