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
    Obj* objects;
} VM;

// VM runs the chunk and then responds...
// ... w/ a value from this enum
typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;

extern VM vm;

// initializes a VM
void initVM(void);

// frees a VM
void freeVM(void);

// pushes a Value to the stack
void push(Value value);

// pops a Value off the stack
Value pop(void);

// interprets a chunk of bytecode
InterpretResult interpret(const char* src);

#endif
