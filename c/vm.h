#ifndef clox_vm_h
#define clox_vm_h

#include "chunk.h"

typedef struct {
    Chunk* chunk;

    // instruction ptr (ip) is defined as a addebyte ptr
    uint8_t* ip;
} VM;

// results of interpret()
typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;

void initVM(VM* vm);
void freeVM(VM* vm);

InterpretResult interpret(Chunk* chunk);

#endif