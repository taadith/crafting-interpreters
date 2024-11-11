#ifndef clox_compiler_h
#define clox_compiler_h

#include "chunk.h"

bool compile(const char* src, Chunk* chunk);

#endif