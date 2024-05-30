#pragma once
#include "tailored.h"

typedef struct t_texture_data {
  unsigned char* bytes;
  unsigned int width;
  unsigned int height;
  unsigned int channels;

} t_texture_data;

typedef struct t_texture {
  unsigned int id;
} t_texture;

t_texture_data t_load_texture_data(const char* path);
t_texture t_load_texture(const char *path);
t_texture t_load_texture_from_data(const t_texture_data* texture_data);
void t_free_texture(t_texture* texture);