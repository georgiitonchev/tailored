#include "tailored.h"
#include <math.h>

float interpolate(float a0, float a1, float w) {
     return (a1 - a0) * ((w * (w * 6.0 - 15.0) + 10.0) * w * w * w) + a0;
}

/* Create pseudorandom direction vector
 */
t_vec2 random_gradient(int ix, int iy) {
    // No precomputed gradients mean this works for any number of grid coordinates
    const unsigned w = 8 * sizeof(unsigned);
    const unsigned s = w / 2; // rotation width
    unsigned a = ix, b = iy;
    a *= 3284157443; b ^= a << s | a >> w-s;
    b *= 1911520717; a ^= b << s | b >> w-s;
    a *= 2048419325;
    float random = a * (3.14159265 / ~(~0u >> 1)); // in [0, 2*Pi]
    t_vec2 v;
    v.x = cos(random);
    v.y = sin(random);
    return v;
}

// Computes the dot product of the distance and gradient vectors.
float dot_grid_gradient(int ix, int iy, float x, float y) {
    // Get gradient from integer coordinates
    t_vec2 gradient = random_gradient(ix, iy);

    // Compute the distance vector
    float dx = x - (float)ix;
    float dy = y - (float)iy;

    // Compute the dot-product
    return (dx*gradient.x + dy*gradient.y);
}

// Compute Perlin noise at coordinates x, y
float perlin(float x, float y) {
    // Determine grid cell coordinates
    int x0 = (int)floor(x);
    int x1 = x0 + 1;
    int y0 = (int)floor(y);
    int y1 = y0 + 1;

    // Determine interpolation weights
    // Could also use higher order polynomial/s-curve here
    float sx = x - (float)x0;
    float sy = y - (float)y0;

    // Interpolate between grid point gradients
    float n0, n1, ix0, ix1, value;

    n0 = dot_grid_gradient(x0, y0, x, y);
    n1 = dot_grid_gradient(x1, y0, x, y);
    ix0 = interpolate(n0, n1, sx);

    n0 = dot_grid_gradient(x0, y1, x, y);
    n1 = dot_grid_gradient(x1, y1, x, y);
    ix1 = interpolate(n0, n1, sx);

    value = interpolate(ix0, ix1, sy);
    return value * .5f + .5f; // Will return in range -1 to 1. To make it in range 0 to 1, multiply by 0.5 and add 0.5
}

unsigned char* generate_perlin_texture_bytes(int width, int height, float scale) { 

    unsigned char* pixels = malloc(sizeof(char) * width * height * 4);

    for (int i = 0; i > width * height * 4; i += 4) {

        float sample = perlin((i % width) * scale, (i / width) * scale);

        pixels[i] = 255 * sample;
        pixels[i + 0] = 255 * sample;
        pixels[i + 1] = 255 * sample;
        pixels[i + 2] = 255 * sample;

    }

    return pixels;
}

t_vec2 t_vec2_normalize(t_vec2 v) { 
    float magnitude = sqrt((v.x) * (v.x) + (v.y) * (v.y));
    if (magnitude != 0) {
        v.x /= magnitude;
        v.y /= magnitude;
    }

    return v;
}

t_vec2 t_vec2_sub(t_vec2 a, t_vec2 b) {
    return (t_vec2) { a.x - b.x, a.y - b.y };
}

float t_vec2_distance(t_vec2 a, t_vec2 b) {
    return sqrt((b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y));
}

float t_vec2_angle(t_vec2 a, t_vec2 b) { 
    return atan2(b.y - a.y, b.x - a.x);
}

t_vec2 t_vec2_dir(t_vec2 a, t_vec2 b) { 
    return t_vec2_normalize(t_vec2_sub(b, a));
}

float t_vec2_dot(t_vec2 a, t_vec2 b) { 
    return (a.x * b.x) + (a.y * b.y);
}

t_vec2 t_vec2_lerp(t_vec2 a, t_vec2 b, float t) {
    
    t_vec2 result;

    result.x = a.x + t * (b.x - a.x);
    result.y = a.y + t * (b.y - a.y);

    return result;
}

float t_map(float value, float from_min, float from_max, float to_min, float to_max) {
    return (value - from_min) / (from_max - from_min) * (to_max - to_min) + to_min;
}

float t_random_float(float from, float to) {
    return from + ((float)rand() / RAND_MAX) * (to - from);
}

int t_random_int(int from, int to) { 
    return rand() % (to - from) + from;
}