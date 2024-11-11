#ifndef clox_compiler_h
#define clox_compiler_h

#include "chunk.h"

// returns whether or not compilation succeeded
// compiler writes opcode into `chunk`
bool compile(const char* src, Chunk* chunk);

#endif