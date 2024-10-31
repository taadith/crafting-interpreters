#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"
#include "value.h"

// each instruction has one-byte operation code (opcode)
// the opcode controls what kind of...
// ... instruction is being dealt w/
typedef enum {
    OP_CONSTANT,
    OP_RETURN
} OpCode;

// struct that holds series of instructions
typedef struct {
    int count;
    int capacity;
    uint8_t* code;
    int* lines;
    ValueArray constants;
} Chunk;

void initChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, uint8_t byte, int line);
void freeChunk(Chunk* chunk);

int addConstant(Chunk* chunk, Value value);

#endif