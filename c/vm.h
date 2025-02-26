#ifndef clox_vm_h
#define clox_vm_h

#include "chunk.h"

typedef struct {
    Chunk* chunk;
    
    // pts to next opcode to be used...
    // ... note: faster to deref a ptr than...
    // ... look up an element in an array by index
    uint8_t* ip; 
} VM;

// VM runs the chunk and then responds...
// ... w/ a value from this enum
typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;

// initializes a VM
void initVM(VM* vm);

// frees a VM
void freeVM(VM* vm);

// interprets a chunk of bytecode
InterpretResult interpret(VM* vm, Chunk* chunk);

#endif
