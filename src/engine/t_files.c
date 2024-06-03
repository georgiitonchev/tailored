#include "t_core.h"
#include "t_files.h"

#include <stdio.h>
#include <stdlib.h>

const char *t_read_file(const char *path, long* file_size) {
  
  char *text_buffer;

  FILE *file_pointer = fopen(path, "rb");

  if (file_pointer == NULL) {
    perror(path);
    return NULL;
  }

  fseek(file_pointer, 0, SEEK_END);
  *file_size = ftell(file_pointer);
  rewind(file_pointer);

  text_buffer = (char *)malloc((*file_size + 1) * sizeof(char));

  fread(text_buffer, sizeof(char), *file_size, file_pointer);

  text_buffer[*file_size] = '\0';

  fclose(file_pointer);

  return text_buffer;
}

t_result t_write_file(const char* path, const char* file_data) {

  FILE *file = fopen(path, "wt");
  if (file != NULL)
  {
      int count = fprintf(file, "%s", file_data);

      if (count < 0) {
        fclose(file);
        printf("ERROR: Could not write to file: %s.\n", path);
        return T_ERROR;
      }

      fclose(file);
      return T_SUCCESS;
  }
  else {
      printf("ERROR: Could not create file: %s.\n", path);
      return T_ERROR;
  }
}
