#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"

// each instruction has one-byte operation code (opcode)
// the opcode controls what kind of...
// ... instruction is being dealt w/
typedef enum {
    OP_RETURN
} OpCode;

// struct that holds series of instructions
typedef struct {
    int count;
    int capacity;
    uint8_t* code;
} Chunk;

void initChunk(Chunk* chunk);
void freeChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, uint8_t byte);

#endif