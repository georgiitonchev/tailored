#pragma once

float t_ease_out_quint(float* timer, float* value, float from, float to, float time);

// Fixed to use for 60 fps
float t_ease_out_quint_f(float* timer, float* value, float from, float to, float time);
float t_ease_in_back(float* timer, float* value, float from, float to, float time);
