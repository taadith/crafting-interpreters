#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"

// one-byte opcodes are defined here
typedef enum {
    OP_RETURN       // returns from current function
} OpCode;

// wrapper around an array of bytes
typedef struct {
    int count;
    int capacity;
    uint8_t* code;
} Chunk;

// initializes a new Chunk
void initChunk(Chunk* chunk);

// writes a byte to a Chunk
void writeChunk(Chunk* chunk, uint8_t byte);

// frees the Chunk
void freeChunk(Chunk* chunk);

#endif
