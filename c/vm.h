#ifndef clox_vm_h
#define clox_vm_h

#include "chunk.h"
#include "value.h"

#define STACK_MAX 256

typedef struct {
    Chunk* chunk;

    // instruction ptr (ip) is defined as a byte ptr
    uint8_t* ip;

    // stack is implemented as a double[]
    Value stack[STACK_MAX];

    // pts just past the last item
    Value* stackTop;
} VM;

// results of interpret()
typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;

void initVM();
void freeVM();

InterpretResult interpret(Chunk* chunk);

void push(Value value);
Value pop();
// Value peek(); ???

#endif