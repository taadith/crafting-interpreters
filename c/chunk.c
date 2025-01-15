#include <stdlib.h>

#include "chunk.h"
#include "memory.h"

// initializes a new Chunk
void initChunk(Chunk* chunk) {
    chunk -> count = 0;
    chunk -> capacity = 0;
    chunk -> code = NULL;
}

// writes a byte to a Chunk
void writeChunk(Chunk* chunk, uint8_t byte) {
    // check if chunk doesn't have capacity for new byte
    if ((chunk -> capacity) < (chunk -> count) + 1) {
        int oldCapacity = chunk -> capacity;
        chunk -> capacity = GROW_CAPACITY(oldCapacity);
        chunk -> code = GROW_ARRAY(uint8_t, chunk -> code,
            oldCapacity, chunk -> capacity);
    }

    // adding new byte to chunk
    chunk -> code[chunk -> count] = byte;

    // incrementing count
    chunk -> count++;
}

// frees the Chunk
void freeChunk(Chunk* chunk) {
    // deallocate all of the memory
    FREE_ARRAY(uint8_t, chunk -> code, chunk -> capacity);

    // zeroes out the fields
    initChunk(chunk);
}
