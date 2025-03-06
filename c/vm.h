#ifndef clox_vm_h
#define clox_vm_h

#include "chunk.h"
#include "value.h"

#define STACK_MAX 256
#define STARTING_STACK_MAX 256

typedef struct {
    Chunk* chunk;
    
    // pts to next opcode to be used...
    // ... note: faster to deref a ptr than...
    // ... look up an element in an array by index
    uint8_t* ip;

    int count; // also serves as where the top of the stack is
    int capacity;

    Value* dyn_stack;
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

// pushes a Value to the stack
void push(VM* vm, Value value);

// pops a Value off the stack
Value pop(VM* vm);

// interprets a chunk of bytecode
InterpretResult interpret(VM* vm, Chunk* chunk);

#endif
