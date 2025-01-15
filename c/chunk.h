#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"

// one-byte opcodes are defined here
typedef enum {
    OP_RETURN       // returns from current function
} OpCode;

// wrapper around an array of bytes
typedef struct {
    uint8_t* code;
} Chunk;

// initializes a new Chunk
void initChunk(Chunk* chunk);

#endif
