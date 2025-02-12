#include <stdlib.h>

#include "chunk.h"
#include "memory.h"

// initializes a chunk
void initChunk(Chunk* chunk) {
    chunk -> count = 0;
    chunk -> capacity = 0;
    chunk -> code = NULL;
    chunk -> lines = NULL;

    // initialize the ValueArray
    initValueArray(&chunk -> constants);
}

// appends a byte to the chunk
void writeChunk(Chunk* chunk, uint8_t byte, int line) {
    // if the array doesn't have capacity for the new byte
    if (chunk -> capacity < chunk -> count + 1) {
        int oldCapacity = chunk -> capacity;
        chunk -> capacity = GROW_CAPACITY(oldCapacity);
        chunk -> code = GROW_ARRAY(uint8_t, chunk -> code,
            oldCapacity, chunk -> capacity);
        chunk -> lines = GROW_ARRAY(int, chunk -> lines,
            oldCapacity, chunk -> capacity);
    }

    // BAU (always going to append a byte)
    chunk -> code[chunk -> count] = byte;
    chunk -> lines[chunk -> count] = line;
    chunk -> count++;
}

// frees the chunk
void freeChunk(Chunk* chunk) {
    // frees code
    FREE_ARRAY(uint8_t, chunk -> code, chunk -> capacity);
    
    // frees lines
    FREE_ARRAY(int, chunk -> lines, chunk -> capacity);

    // frees constants (ValueArray)
    freeValueArray(&chunk -> constants);


    // zeroes out the values, no dangling ptrs
    initChunk(chunk);
}

// add a constant to the chunk
int addConstant(Chunk* chunk, Value value) {
    writeValueArray(&chunk -> constants, value);
    
    // return index where constant was appended...
    // ... to locate later
    return chunk -> constants.count - 1;
}
