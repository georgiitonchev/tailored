#pragma once

typedef struct t_memory_pool {
    void* pool;
    unsigned int pool_size;
    unsigned int used;

} t_memory_pool;

typedef struct t_list {
    void* elements;
    unsigned int capacity;
    unsigned int size;
    unsigned int element_size;

    t_memory_pool* memory_pool;

} t_list;

t_list* create_list(unsigned int element_size);
void destroy_list(t_list* list);
void clear_list(t_list* list);
void add_to_list(t_list* list, const void* element);
void remove_from_list(t_list* list, unsigned int index_of_element);
void remove_from_list_e(t_list* list, const void* element);

void* element_at_list(t_list* list, unsigned int index);