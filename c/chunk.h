#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"
#include "value.h"

// defines our one-byte opcodes (operation codes)
typedef enum {
    OP_CONSTANT,
    OP_RETURN
} OpCode;

typedef struct {
    int count;
    int capacity;

    // dynamic array of bytecode
    uint8_t* code;  // uint8_t is 8 bits == 1 byte
    
    // parallels the bytecode to say what line it's on
    int* lines;
    ValueArray constants;
} Chunk;

// initializes a chunk
void initChunk(Chunk* chunk);

// appends a byte to the chunk
void writeChunk(Chunk* chunk, uint8_t byte, int line);

// frees the chunk
void freeChunk(Chunk* chunk);

// add a constant to the chunk
int addConstant(Chunk* chunk, Value value);

#endif
