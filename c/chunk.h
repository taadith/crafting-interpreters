#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"
#include "rle.h"
#include "value.h"

// each instruction has one-byte operation code (opcode)
// the opcode controls what kind of
// instruction is being dealt w/
typedef enum {
    OP_CONSTANT,
    OP_NIL,
    OP_TRUE,
    OP_FALSE,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NEGATE,
    OP_RETURN
} OpCode;

// struct that holds series of instructions
typedef struct {
    int count;
    int capacity;
    uint8_t* code;
    RunLengthEncoding rle;
    ValueArray constants;
} Chunk;


// creates a chunk
void initChunk(Chunk* chunk);

// writes an OpCode to a chunk
void writeChunk(Chunk* chunk, uint8_t byte, int line);

// frees a chunk
void freeChunk(Chunk* chunk);

// adds a constant value to a chunk
int addConstant(Chunk* chunk, Value value);

#endif