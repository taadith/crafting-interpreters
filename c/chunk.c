#include <stdlib.h>

#include "chunk.h"
#include "memory.h"

// initializes a chunk
void initChunk(Chunk* chunk) {
    chunk -> count = 0;
    chunk -> capacity = 0;
    chunk -> code = NULL;
}

// appends a byte to the chunk
void writeChunk(Chunk* chunk) {
    // if the array doesn't have capacity for the new byte
    if (chunk -> capacity < chunk -> count + 1) {
        int oldCapacity = chunk -> capacity;
        chunk -> capacity = GROW_CAPACITY(oldCapacity);
        chunk -> code = GROW_ARRAY(uint8_t, chunk -> code,
            oldCapacity, chunk -> capacity);
    }

    // BAU (always going to append a byte)
    chunk -> code[chunk -> count] = byte;
    chunk -> count++;
}
