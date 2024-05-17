#include "screens.h"
#include "./engine/tailored.h"

#include "game.h"

#include <stdio.h>

static float s_splash_timer = 0;

static t_sprite s_sprite_splash_logo;
static t_vec2 s_window_size;

static float s_timer_splash_fade = 0;
static float s_splash_fade = 0;

int load_splash_screen(void* args) {
     UNUSED(args);

     t_load_texture_data_s(&s_sprite_splash_logo, "./res/textures/splash_logo.png");

     set_loading_progress(1);
     set_loading_finished();
     return 0;
}

void init_splash_screen() {
     t_init_sprite(&s_sprite_splash_logo);
     s_window_size = t_window_size();
}

void unload_splash_screen() {
     t_deinit_sprite(&s_sprite_splash_logo);
}

void draw_splash_screen() {

     t_clear_color(CC_BLACK);

     t_color logo_color = CC_RED;
     logo_color.a -= CC_RED.a * s_splash_fade;

     if (s_splash_timer > 1)
          t_ease_in_back(&s_timer_splash_fade, &s_splash_fade, 0, 1, 2.5f);

     t_draw_sprite(&s_sprite_splash_logo, (s_window_size.x - s_sprite_splash_logo.texture_data.width) / 2, (s_window_size.y - s_sprite_splash_logo.texture_data.height) / 2, 
          s_sprite_splash_logo.texture_data.width, s_sprite_splash_logo.texture_data.height, logo_color);

     if (s_splash_timer >= 4) {
          set_screen(TITLE);
     }

     s_splash_timer += t_delta_time();
}