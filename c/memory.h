#ifndef clox_memory_h
#define clox_memory_h

#include "common.h"

// array starts at default size of 8, then doubles
#define GROW_CAPACITY(capacity) \
    ((capacity) < 8 ? 8 : (capacity) * 2)

#define GROW_ARRAY(type, ptr, oldCount, newCount) \
    (type*)reallocate(ptr, sizeof(type) * (oldCount), \
        sizeof(type) * (newCount))

#define FREE_ARRAY(type, ptr, oldCount) \
    reallocate(ptr, sizeof(type) * (oldCount), 0)

void* reallocate(void* ptr, size_t oldSize, size_t newSize);

#endif