#include <stdlib.h>

#include "memory.h"

void* reallocate(void* ptr, size_t newSize) {
    // realloc doesn't handle freeing a ptr
    if (newSize == 0) {
        free(ptr);
        return NULL;
    }

    // nifty for the other three cases
    void* result = realloc(ptr, newSize);

    // allocation failed b/c there...
    // ... wasn't enough memory
    if (result == NULL)
        exit(1);

    return result;
}
