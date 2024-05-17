#include "tailored.h"

// STB
#define STB_IMAGE_IMPLEMENTATION
#include "../../dep/include/stb/stb_image.h"

t_texture_data t_load_texture_data(const char* path) {
  int width, height, channels;
  unsigned char *bytes = stbi_load(path, &width, &height, &channels, 0);

  if (bytes == NULL) {
    printf("error loading texture: %s\n", path);
  }

  t_texture_data texture_data = {
    .bytes = bytes,
    .channels = channels,
    .width = width,
    .height = height
  };

  return texture_data;
}

static t_texture s_load_texture(const t_texture_data* texture_data) {

  GLenum texture_format = 0;
  if (texture_data->channels == 1)
    texture_format = GL_RED;
  else if (texture_data->channels == 3)
    texture_format = GL_RGB;
  else if (texture_data->channels == 4)
    texture_format = GL_RGBA;

  unsigned int texture_id;
  
  glGenTextures(1, &texture_id);
  glBindTexture(GL_TEXTURE_2D, texture_id);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glTexImage2D(GL_TEXTURE_2D, 0, texture_format, texture_data->width, texture_data->height, 0,
              texture_format, GL_UNSIGNED_BYTE, texture_data->bytes);

  glBindTexture(GL_TEXTURE_2D, 0);

  stbi_image_free(texture_data->bytes);

  t_texture texture;
  texture.id = texture_id;

  return texture;
}

t_texture t_load_texture_from_data(const t_texture_data* texture_data) {
  return s_load_texture(texture_data);
}

t_texture t_load_texture(const char *texture_path) {

  t_texture_data texture_data = t_load_texture_data(texture_path);
  return s_load_texture(&texture_data);
}

void t_free_texture(t_texture* texture) 
{
    glDeleteTextures(1, &texture->id);
}
