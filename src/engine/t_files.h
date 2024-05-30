#pragma once
#include "t_core.h"

const char *t_read_file(const char *path, long* file_size);
t_result t_write_file(const char* path, const char* file_data);