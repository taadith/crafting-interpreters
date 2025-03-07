#include <stdio.h>

#include "common.h"
#include "debug.h"
#include "vm.h"

// initializes a VM
void initVM(VM* vm) {
    vm -> count = 0;
    vm -> capacity = 0;
    vm -> dyn_stack = NULL;
}

// frees a VM
void freeVM(VM* vm) {
    vm -> count = 0;
    vm -> capacity = 0;

    NEW_FREE_ARRAY(vm -> dyn_stack);
}

// pushes a Value to the stack
void push(VM* vm, Value value) {
    if (vm -> capacity < vm -> count + 2) {
        int oldCapacity = vm -> capacity;
        vm -> capacity = GROW_CAPACITY(oldCapacity);

        vm -> dyn_stack = NEW_GROW_ARRAY(Value, vm -> dyn_stack,
            vm -> capacity);
    }

    vm -> dyn_stack[vm -> count] = value;
    vm -> count++;
}

// pops a Value off the stack
Value pop(VM* vm) {
    // change where top of stack is
    vm -> count--;

    return vm -> dyn_stack[vm -> count];
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
        for(int i = 0; i < vm -> count; i++) {
            printf("[ ");
            printValue(vm -> dyn_stack[i]);
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

InterpretResult interpret(const char* src) {
    compile(src);
    return INTERPRET_OK;
}
