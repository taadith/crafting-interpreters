#ifndef clox_vm_h
#define clox_vm_h

#include "chunk.h"
#include "table.h"
#include "value.h"

#define STACK_MAX 256

typedef struct {
    Chunk* chunk;

    // instruction ptr (ip) is defined as a byte ptr
    // pts to address of current bytecode instruction
    uint8_t* ip;

    // stack is implemented as a double[]
    Value stack[STACK_MAX];

    // pts just past the last item
    Value* stackTop;

    // hash table of global variables
    Table globals;

    // hash table (more like hash set)...
    // ... of "interned" strings
    Table strings;

    // VM stores a ptr to the...
    // ... head of the linked list
    Obj* objects;
} VM;

void push(Value value);
Value pop(void);

// results of interpret()
typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;

// exposes global vm variables to other modules
extern VM vm;

void initVM(void);
void freeVM(void);

InterpretResult interpret(const char* src);

#endif
