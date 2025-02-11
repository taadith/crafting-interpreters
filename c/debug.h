#ifndef clox_debug_h
#define clox_debug_h

#include "chunk.h"

// disassembles all the instructions in the chunk
void disassembleChunk(Chunk* chunk, const char* name);

// disassembles 
int disassembleInstruction(Chunk* chunk, int offset);
