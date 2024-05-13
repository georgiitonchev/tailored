#include "t_list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 32
#define INITIAL_POOL_SIZE 100000

static t_memory_pool* create_memory_pool(unsigned int pool_size) {

     if (pool_size == 0) {
        printf("ERROR: t_memory_pool size can't be zero.\n");
        return NULL;
    }

    t_memory_pool* memory_pool = (t_memory_pool*)malloc(sizeof(t_memory_pool));
    if (!memory_pool) {
        printf("ERROR: Could not allocate memory for t_memory_pool struct.\n");
        return NULL;
    }

    memory_pool->pool = malloc(pool_size);
    if (!memory_pool->pool) {
        printf("ERROR: Could not allocate memory for pool.\n");
        free(memory_pool);
        return NULL;
    }

    memory_pool->pool_size = pool_size;
    memory_pool->used = 0;

    return memory_pool;
}

static void* allocate_from_memory_pool(t_memory_pool* memory_pool, unsigned int size) {

    // get pointer to end of pool
    void* memory = (char*)memory_pool->pool + memory_pool->used;

    // update end of pool
    memory_pool->used += size;

    return memory;
}

static void destroy_memory_pool(t_memory_pool* memory_pool) {
    free(memory_pool->pool);
    free(memory_pool);
}

t_list* create_list(unsigned int element_size) {
    
    t_list* list = (t_list*)malloc(sizeof(t_list));

    if (!list) {
        printf("ERROR: Could not allocate memory for t_list struct.\n");
        exit(-1);
    }

    list->size = 0;
    list->capacity = INITIAL_CAPACITY;
    list->element_size = element_size;

    list->memory_pool = create_memory_pool(INITIAL_POOL_SIZE);
    if (!list->memory_pool) { 
        free(list);
        exit(-1);
    }

    list->elements = allocate_from_memory_pool(list->memory_pool, list->capacity * element_size);
    if (!list->elements) {
        printf("ERROR: Could not allocate memory for list elements.\n");
        destroy_memory_pool(list->memory_pool);
        free(list);
        exit(-1);
    }

    return list;
}

void add_to_list(t_list* list, const void* element) {

    if (!list) { 
        printf("ERROR: Tried to add to an uninitialized list.\n");
        return;
    }

    if (list->size >= list->capacity) {
        unsigned int updated_capacity = list->capacity * 2;

        // allocate new momory for resized list
        void* elements_memory = allocate_from_memory_pool(list->memory_pool, updated_capacity * list->element_size);
        if (!elements_memory) {
            printf("ERROR: Could not allocate memory for additional elements.\n");
            return;
        }

        // copy existing items to new memory
        memcpy(elements_memory, list->elements, list->size * list->element_size);
        list->elements = elements_memory;
        list->capacity = updated_capacity;
    }

    // add new element
    memcpy((char*)list->elements + (list->size * list->element_size), element, list->element_size);
    list->size++;
}

void remove_from_list(t_list* list, unsigned int index_of_element) {

    if (index_of_element < list->size - 1) { 
        memcpy((char*)list->elements + (index_of_element * list->element_size), 
            (char*)list->elements + ((index_of_element + 1) * list->element_size), list->element_size * (list->size - (index_of_element + 1)));
    }

    list->size--;
}

void* element_at_list(t_list* list, unsigned int index) {

    if (!list) {
        printf("ERROR: list is NULL at element_at_list.\n");
        return NULL;
    }

    if (index >= list->size) {
        printf("ERROR: index is out of bounds.\n");
        return NULL;
    }

    void* element = (char*)list->elements + (list->element_size * index);
    return element;
}

void destroy_list(t_list* list) {
    destroy_memory_pool(list->memory_pool);
    free(list);
}