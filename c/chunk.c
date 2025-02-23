#include <stdlib.h>

#include "chunk.h"
#include "memory.h"

// initializes a chunk
void initChunk(Chunk* chunk) {
    chunk -> count = 0;
    chunk -> capacity = 0;
    chunk -> code = NULL;
    
    // initializes the RLE for lines
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
    }

    // BAU (always going to append a byte)
    chunk -> code[chunk -> count] = byte;

    // adding a line
    writeRunLengthEncoding(&chunk -> rle_lines, line);

    chunk -> count++;
}

// frees the chunk
void freeChunk(Chunk* chunk) {
    // frees code
    FREE_ARRAY(uint8_t, chunk -> code, chunk -> capacity);
    
    // frees lines
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

// adds a constant to the constant pool...
// ... and returns its index
int addConstant(Chunk* chunk, Value value) {
    writeValueArray(&chunk -> constants, value);
    
    // return index where constant was appended...
    // ... to locate later
    return chunk -> constants.count - 1;
}

// writes an appropriate constant opcode to the chunk...
// ... and the value's index appropriately
void writeConstant(Chunk* chunk, Value value, int line) {
    // adding the value to the constant pool...
    // ... and grabbing it's index
    int valueIndex = addConstant(chunk, value);

    // writing an OP_CONSTANT
    if (valueIndex < 256) {
        // write the opcode and valueIndex
        writeChunk(chunk, OP_CONSTANT, line);
        writeChunk(chunk, valueIndex, line);
    }

    // writing an OP_CONSTANT_LONG
    else if (valueIndex >= 256) {
        // we know the index, which needs to be split...
        // ... into three bytes
        int first_byte = (valueIndex & (255 << 16)) >> 16;
        int second_byte = (valueIndex & (255 << 8)) >> 8;
        int third_byte = valueIndex & 255;

        // write the opcode and split up valueIndex
        writeChunk(chunk, OP_CONSTANT_LONG, line);
        writeChunk(chunk, first_byte, line);
        writeChunk(chunk, second_byte, line);
        writeChunk(chunk, third_byte, line);
    }

}
