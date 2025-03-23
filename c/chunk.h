#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"
#include "rle.h"
#include "value.h"

// defines our one-byte opcodes (operation codes)
typedef enum {
    OP_CONSTANT,
    OP_CONSTANT_LONG,
    OP_NIL,
    OP_TRUE,
    OP_FALSE,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NOT,
    OP_NEGATE,
    OP_RETURN
} OpCode;

typedef struct {
    int count;
    int capacity;

    // dynamic array of bytecode
    uint8_t* code;  // uint8_t is 8 bits == 1 byte
    
    // parallels the bytecode to say what line it's on
    RunLengthEncoding rle_lines;

    // stores the constant pool
    ValueArray constants;
} Chunk;

// initializes a chunk
void initChunk(Chunk* chunk);

// appends a byte to the chunk
void writeChunk(Chunk* chunk, uint8_t byte, int line);

// frees the chunk
void freeChunk(Chunk* chunk);

// grab the line value at index of rle_lines
int getLine(Chunk* chunk, int offset);

// adds a constant to the constant pool...
// ... and returns its index
int addConstant(Chunk* chunk, Value value);

// writes an OP_CONSTANT_LONG to the chunk
void writeConstant(Chunk* chunk, Value value, int line);

#endif
