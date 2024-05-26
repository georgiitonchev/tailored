#include "screen_title.h"

#include "../../engine/tailored.h"
#include "../../../dep/include/glad/glad.h"
#include "../../game.h"

// FONTS
static t_font s_font_ui_s;

// SPRITES
static t_sprite s_sprite_section_background;
static t_sprite s_sprite_slider_background;
static t_sprite s_sprite_small_knob;
static t_sprite s_sprite_big_knob;

// BUTTONS
static t_ui_button s_button_slider_knob;

// STATE
static float s_value_scrolled = 0;

static t_rect s_rect_slider;
static float s_text_height;

static const char* s_text_about = 
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit,"
    " sed do eiusmod tempor incididunt ut labore et dolore magna aliqua."
    " Dictum sit amet justo donec. Faucibus nisl tincidunt eget nullam non."
    " Egestas tellus rutrum tellus pellentesque eu tincidunt. Donec enim diam"
    " vulputate ut pharetra sit amet aliquam id. Quis viverra nibh cras pulvinar mattis nunc sed blandit."
    " Posuere sollicitudin aliquam ultrices sagittis orci a scelerisque purus. In ornare quam viverra orci."
    " Congue quisque egestas diam in arcu. Tempus urna et pharetra pharetra massa."
    " Dolor magna eget est lorem ipsum dolor. Ut etiam sit amet nisl purus in mollis nunc sed."
    " Cras fermentum odio eu feugiat pretium nibh ipsum consequat nisl."
    " Consectetur purus ut faucibus pulvinar elementum. Elementum eu facilisis sed odio morbi.";

static void s_on_slider_knob_button_pressed() { 

    float value = (mouse_position().y - (s_rect_slider.y + s_button_slider_knob.sprite->texture_data.height / 2)) 
        / (s_rect_slider.height - s_button_slider_knob.sprite->texture_data.height);

    if (value < 0)
        value = 0;
    else if (value > 1)
        value = 1;

    s_value_scrolled = value;
}

void load_section_about() {

    t_load_texture_data_s(&s_sprite_section_background, "./res/textures/panel-transparent-center-030.png");
    t_load_texture_data_s(&s_sprite_slider_background, "./res/textures/slider_background_v.png");
    t_load_texture_data_s(&s_sprite_small_knob, "./res/textures/slider_knob_small_v.png");
    t_load_texture_data_s(&s_sprite_big_knob, "./res/textures/slider_knob_big_v.png");
}

void init_section_about() {

    s_font_ui_s = load_ttf_font("./res/fonts/Eczar-Regular.ttf", 32);

    t_init_sprite(&s_sprite_section_background);
    s_sprite_section_background.slice_borders = (t_vec4){ 16, 16, 16, 16 };

    t_init_sprite(&s_sprite_slider_background);
    t_init_sprite(&s_sprite_small_knob);
    t_init_sprite(&s_sprite_big_knob);

    s_button_slider_knob = create_ui_button(&s_sprite_big_knob);
    s_button_slider_knob.color_default = CC_BLACK;
    s_button_slider_knob.color_mouseover = CC_DARK_RED;
    s_button_slider_knob.color_clicked = CC_DARK_RED;
    s_button_slider_knob.on_pressed = s_on_slider_knob_button_pressed;

    s_text_height = measure_text_size_w(s_text_about, &s_font_ui_s, CC_UI_SECTION_RECT.w - 32).y;
    t_log_debug("Text height: %f", s_text_height);
}

void draw_section_about(const float p_offset_x, const float p_offset_y) {

    t_draw_sprite(&s_sprite_section_background, CC_UI_SECTION_RECT.x + p_offset_x, CC_UI_SECTION_RECT.y + p_offset_y, CC_UI_SECTION_RECT.z, CC_UI_SECTION_RECT.w, CC_LIGHT_RED);
    t_begin_scissor(CC_UI_SECTION_RECT.x + CC_UI_PADDING.left, CC_UI_SECTION_RECT.y + CC_UI_PADDING.top + p_offset_y, CC_UI_SECTION_RECT.z - 48, CC_UI_SECTION_RECT.w - 32);
        draw_text_ttf(s_text_about, &s_font_ui_s, (t_vec2) { 256 + 16 , 16 + 32 + p_offset_y - s_value_scrolled * (s_text_height - CC_UI_SECTION_RECT.w - 32)  }, CC_BLACK, CC_UI_SECTION_RECT.w - 32);
    t_end_scissor();

    s_rect_slider = (t_rect) { 597, 32 + p_offset_y, s_sprite_slider_background.texture_data.width, 296};

    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glClear(GL_STENCIL_BUFFER_BIT);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);
    draw_ui_button(&s_button_slider_knob, 595, 32 + p_offset_y + s_value_scrolled * (s_rect_slider.height - s_button_slider_knob.sprite->texture_data.height), s_button_slider_knob.sprite->texture_data.width, s_button_slider_knob.sprite->texture_data.height);
    
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    if (!s_button_slider_knob.is_mouse_over && is_point_in_rect(mouse_position(), s_rect_slider)) {

        if (is_mouse_button_pressed(MOUSE_BUTTON_LEFT)) {
            s_button_slider_knob.was_clicked = true;
            s_button_slider_knob.mouse_clicked_at = (t_vec2) { s_button_slider_knob.sprite->texture_data.width / 2, s_button_slider_knob.sprite->texture_data.height / 2};
        }

        t_rect rect_small_knob = (t_rect) { s_rect_slider.x, mouse_position().y - s_sprite_small_knob.texture_data.height / 2 + p_offset_y, s_sprite_small_knob.texture_data.width, s_sprite_small_knob.texture_data.height };

        if (rect_small_knob.y < s_rect_slider.y)
            rect_small_knob.y = s_rect_slider.y;
        else if (rect_small_knob.y > s_rect_slider.y + s_rect_slider.height- rect_small_knob.height)
            rect_small_knob.y = s_rect_slider.y + s_rect_slider.height - rect_small_knob.height;

        t_draw_sprite_r(&s_sprite_small_knob, rect_small_knob, CC_BLACK);
    }

    glStencilMask(0x00);

    t_draw_sprite_r(&s_sprite_slider_background, s_rect_slider, CC_BLACK);
    glStencilMask(0xFF);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
}

void unload_section_about() {

    t_deinit_sprite(&s_sprite_section_background);
    t_deinit_sprite(&s_sprite_slider_background);
    t_deinit_sprite(&s_sprite_small_knob);
    t_deinit_sprite(&s_sprite_big_knob);

    delete_ttf_font(&s_font_ui_s);
}