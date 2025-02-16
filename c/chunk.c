#include <stdlib.h>

#include "chunk.h"
#include "memory.h"

// initializes a chunk
void initChunk(Chunk* chunk) {
    chunk -> count = 0;
    chunk -> capacity = 0;
    chunk -> code = NULL;
    
    // initializes the RLE for lines
    chunk -> lines = NULL;
    initRunLengthEncoding(&chunk -> rle_lines);

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

    // adding a line
    chunk -> lines[chunk -> count] = line;
    writeRunLengthEncoding(&chunk -> rle_lines, line);

    chunk -> count++;
}

// frees the chunk
void freeChunk(Chunk* chunk) {
    // frees code
    FREE_ARRAY(uint8_t, chunk -> code, chunk -> capacity);
    
    // frees lines
    FREE_ARRAY(int, chunk -> lines, chunk -> capacity);
    freeRunLengthEncoding(&chunk -> rle_lines);

    // frees constants (ValueArray)
    freeValueArray(&chunk -> constants);


    // zeroes out the values, no dangling ptrs
    initChunk(chunk);
}

// grab the line value at index of rle_lines
int getLine(Chunk* chunk, int offset) {
    return getValueAtIndex(&chunk -> rle_lines, offset);
}

// add a constant to the chunk
int addConstant(Chunk* chunk, Value value) {
    writeValueArray(&chunk -> constants, value);
    
    // return index where constant was appended...
    // ... to locate later
    return chunk -> constants.count - 1;
}
