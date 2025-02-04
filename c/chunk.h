#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"

// defines our one-byte opcodes (operation codes)
typedef enum {
    OP_RETURN
} OpCode;

typedef struct {
    int count;
    int capacity;
    // dynamic array of bytecode
    uint8_t* code;  // uint8_t is 8 bits == 1 byte
} Chunk;

void initChunk(Chunk* chunk);

#endif
