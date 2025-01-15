#ifndef clox_memory_h
#define clox_memory_h

#include "common.h"

// calculates a new capacity based...
// ... off the old capacity
#define GROW_CAPACITY(capacity) \
    ((capacity) < 8 ? 8 : (capacity) * 2)

// pretties up reallocate() where...
// ... the real work happens
#define GROW_ARRAY(type, ptr, oldCount, newCount) \
    (type*) reallocate(ptr, sizeof(type) * (newCount))

// another wrapper around reallocate()
#define FREE_ARRAY(ptr) \
    reallocate(ptr, 0)

void* reallocate(void* ptr, size_t newSize);

#endif
