#include "t_easings.h"

#include "tailored.h"
#include "math.h"

static float ease_out_quint(float x) {
    return 1 - pow(1 - x, 5);
}

float t_ease_out_quint(float* timer, float* value, float from, float to, float time) {

    *timer += t_delta_time() * (1 / time);

    *value =
        from +
        (to - from) *
        ease_out_quint(*timer);

    return *timer;
}