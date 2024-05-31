#include "t_array.h"


static void s_resize_array(t_array* array, size_t capacity) {
	void** data = realloc(array->data, capacity * sizeof(void*));

	if (data == NULL) {
		printf("Could not realloc when resizing t_array->data.\n");
		exit(EXIT_FAILURE);
	}

	array->data = data;
	array->capacity = capacity;
}

t_array* create_array() {

	t_array* array = malloc(sizeof(t_array));

	if (array == NULL) {
		printf("Could not malloc for t_array.\n");

		exit(EXIT_FAILURE);
	}

	array->data = malloc(32 * sizeof(void*));

	if (array->data == NULL) {
		printf("Could not malloc for t_array->data.\n");

		free(array);
		exit(EXIT_FAILURE);
	}

	array->size = 0;
	array->capacity = 32;
	return array;
}

void destroy_array(t_array* array) {
	free(array->data);
	free(array);
}

void add_to_array(t_array* array, void* element) {

	if (array->size == array->capacity)
		s_resize_array(array, array->capacity * 2);

	array->data[array->size++] = element;
}

void remove_from_array_at_index(t_array* array, size_t index) {

	if (index >= array->size) {
		printf("Index of array larger than size at remove_from_array_at_index. \n");
		return;
	}

	for (size_t i = index; i < array->size - 1; ++i) {
		array->data[i] = array->data[i + 1];
	}

	array->size--;
}

void remove_from_array(t_array* array, void* element) {

	for (size_t i = 0; i < array->size; ++i) {
		if (array->data[i] == element) {
			remove_from_array_at_index(array, i);
			return;
		}
	}
}

void* element_at_array(t_array* array, int index) {

	 if (!array) {
        printf("ERROR: array is NULL at element_at_array.\n");
        return NULL;
    }

    if (index >= array->size) {
        printf("ERROR: Array index is out of bounds.\n");
        return NULL;
    }

    return array->data[index];
}

void clear_array(t_array* array) {
	array->size = 0;
}
