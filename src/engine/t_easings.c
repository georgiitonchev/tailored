#include "tailored.h"
#include "math.h"

static float ease_out_quint(float x) {
    return 1 - pow(1 - x, 5);
}

float t_ease_out_quint(float* timer, float* value, float from, float to, float time) {

    *timer += t_delta_time() * (1.0f / time);

    *value =
        from +
        (to - from) *
        ease_out_quint(*timer);

    return *timer;
}

float t_ease_out_quint_f(float* timer, float* value, float from, float to, float time) { 

    *timer += (1.0f / 60.0f) * (1.0f / time);

    *value =
        from +
        (to - from) *
        ease_out_quint(*timer);

    return *timer;
}

static float ease_in_back(float x) { 
    const float c = 1.70158f;
    const float cc = c + 1;

    return cc * x * x * x - c * x * x;
}

float t_ease_in_back(float* timer, float* value, float from, float to, float time) {

    *timer += t_delta_time() * (1.0f / time);

    *value =
        from +
        (to - from) *
        ease_in_back(*timer);

    return *timer;
}