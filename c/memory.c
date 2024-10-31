#include <stdlib.h>

#include "memory.h"

void* reallocate(void* ptr, size_t oldSize, size_t newSize) {
    // oldSize = 0 && newSize > 0
    if (newSize == 0) {
        free(ptr);
        return NULL;
    }

    // handles all other cases!
    // tries to change the size of the allocation...
    // ... pointed to by ptr and returns ptr
    void* result = realloc(ptr, newSize);

    // failed allocation
    if (result == NULL)
        exit(1);
    
    return result;
}