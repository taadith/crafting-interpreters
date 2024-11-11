#include <stdlib.h>

#include "chunk.h"
#include "memory.h"

// dynamic array starts off empty
void initChunk(Chunk* chunk) {
    chunk -> count = 0;
    chunk -> capacity = 0;
    chunk -> code = NULL;
    initRunLengthEncoding(&chunk -> rle);
    initValueArray(&chunk -> constants);
}

void writeChunk(Chunk* chunk, uint8_t byte, int line) {
    if (chunk -> capacity < chunk -> count + 1) {
        int oldCapacity = chunk -> capacity;

        // double capacity (or set to 8)
        chunk -> capacity = GROW_CAPACITY(oldCapacity);

        // double size of arrays (or set to 8)
        chunk -> code = GROW_ARRAY(uint8_t, chunk -> code,
                                   oldCapacity, chunk -> capacity);
    }

    chunk -> code[chunk -> count] = byte;
    writeRunLengthEncoding(&chunk -> rle, line);
    chunk -> count++;
}

void freeChunk(Chunk* chunk) {
    FREE_ARRAY(uint8_t, chunk -> code, chunk -> capacity);
    freeRunLengthEncoding(&chunk -> rle);
    freeValueArray(&chunk -> constants);
    
    // zeroing out the values
    initChunk(chunk);
}

// adds the given value to the end of the... 
// ... chunk's constant table and returns its index
int addConstant(Chunk* chunk, Value value) {
    writeValueArray(&chunk -> constants, value);

    // returning the idx where the constant was...
    // ...appended to locate the constant later
    return chunk -> constants.count - 1;
}