#include "t_engine.h"
#include "t_core.h"
#include "t_font.h"

int main() {

    t_begin(640, 360, "Hello");
    while(t_loop()) {

        draw_text("Hello?", (t_vec2) {50, 50}, 18, RED);
        draw_text("Hellooo?", (t_vec2) {50, 150}, 18, RED);

        t_loop_end();
    }
    t_end();
    return 0;
}
