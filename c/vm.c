#include <stdio.h>

#include "common.h"
#include "debug.h"
#include "vm.h"

// TODO: replace static VM with dynamic VM!
// VM vm;

static void resetStack(VM* vm) {
    vm -> stackTop = vm -> stack;
}


// initializes a VM
void initVM(VM* vm) {
    resetStack(vm);
}

// frees a VM
void freeVM(VM* vm) {

}

// pushes a Value to the stack
void push(VM* vm, Value value) {
    *(vm -> stackTop) = value;
    vm -> stackTop++;
}

// pops a Value off the stack
Value pop(VM* vm) {
    // change where top of stack is
    (vm -> stackTop)--;
    return *(vm -> stackTop);
}

// beating heart of VM..
// ... interpreter spends ~90% of time here
static InterpretResult run(VM* vm) {
    // reads byte currently pointed @ by `ip`...
    // ... and then advances `ip`
    #define READ_BYTE() (*(vm -> ip)++)

    // reads next byte from bytecode, treating...
    // ... resulting # as an index, and looks up the...
    // ... corresponding Value in the chunk's constant table
    #define READ_CONSTANT() (vm -> chunk -> \
        constants.values[READ_BYTE()])
    
    // do-while format allows code to be placed...
    // ... w/in a block (in the same scope)...
    #define BINARY_OP(op) \
        do { \
            double b = pop(vm); \
            double a = pop(vm); \
            push(vm, a op b); \
        } while (false)

    for(;;) {
        // diagnostic logging for VM...
        // ... stack trace...
        // ... and disassembling instructions
        #ifdef DEBUG_TRACE_EXECUTION
        printf("\t\t");
        for(Value* slot = vm -> stack; slot < vm -> stackTop; slot++) {
            printf("[ ");
            printValue(*slot);
            printf(" ]");
        }
        printf("\n");

        disassembleInstructionWithRLE(vm -> chunk,
            (int) (vm -> ip - vm -> chunk -> code));
        #endif

        uint8_t instruction;
        switch (instruction = READ_BYTE()) {
            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                push(vm, constant);
                break;
            }

            // TODO: work on OP_CONSTANT_LONG
            
            case OP_ADD: {
                BINARY_OP(+);
                break;
            }

            case OP_SUBTRACT: {
                BINARY_OP(-);
                break;
            }

            case OP_MULTIPLY: {
                BINARY_OP(*);
                break;
            }

            case OP_DIVIDE: {
                BINARY_OP(/);
                break;
            }

            case OP_NEGATE: {
                push(vm, -pop(vm));
                break;
            }

            case OP_RETURN: {
                printValue(pop(vm));
                printf("\n");
                return INTERPRET_OK;
            }
        }
    }

    #undef READ_BYTE
    #undef READ_CONSTANT
    #undef BINARY_OP
}

InterpretResult interpret(VM* vm, Chunk* chunk) {
    vm -> chunk = chunk;
    vm -> ip = (vm -> chunk) -> code;
    return run(vm);
}
