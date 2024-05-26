#pragma once

#include <stdio.h>
#include <stdlib.h>

typedef struct t_array {
	void** data;
	size_t size;
	size_t capacity;

} t_array;

t_array* create_array();
void destroy_array(t_array* array);
void add_to_array(t_array* array, void* element);
void remove_from_array_at_index(t_array* array, size_t index);
void remove_from_array(t_array* array, void* element);
void* element_at_array(t_array* array, int index);
void clear_array(t_array* array);