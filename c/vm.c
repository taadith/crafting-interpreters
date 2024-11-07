#include <stdio.h>

#include "common.h"
#include "debug.h"
#include "vm.h"

VM vm;

static void resetStack() {
    // set stackTop to stack[0]
    vm.stackTop = vm.stack;
}

void initVM() {
    resetStack();
}

void freeVM() {}

static InterpretResult run() {
// reads current byte pointed at by ip...
// ... and then advances the ip
#define READ_BYTE() (*vm.ip++)

// reads the next byte from the bytecode...
// ... treats the resulting # as an index...
// ... and looks up the corresponding Value in the chunk's constant table
#define READ_CONSTANT() (vm.chunk -> constants.values[READ_BYTE()])

// pop values off the stack...
// ... and then push the result
#define BINARY_OP(op) \
    do { \
        double b = pop(); \
        double a = pop(); \
        push(a op b); \
    } while (false)

    for(;;) {

// dynamic debugging if flag is defined
#ifdef DEBUG_TRACE_EXECUTION
        printf("\t\t");
        
        // print each value in the array
        for (Value* slot = vm.stack; slot < vm.stackTop; slot++) {
            printf("[ ");
            printValue(*slot);
            printf(" ]");
        }
        printf("\n");

        disassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk -> code));
#endif

        // first byte of any instruction is the opcode
        uint8_t instruction;
        switch(instruction = READ_BYTE()) {
            case OP_CONSTANT: {
                // constant gets pushed to the stack
                Value constant = READ_CONSTANT();
                push(constant);

                break;
            }

            case OP_NEGATE: {
                push(-pop());
                break;
            }
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
            case OP_RETURN: {
                // return pops the stack and prints...
                // ... the top value before exiting
                printValue(pop());
                printf("\n");

                return INTERPRET_OK;
            }
        }
    }

#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}

InterpretResult interpret(Chunk* ch) {
    vm.chunk = ch;

    // ip pts to the first byte of code in the chunk
    // ip pts to the instruction abt to be executed
    vm.ip = vm.chunk -> code;
    return run();
}

void push(Value value) {
    // set stackTop location to new value
    *(vm.stackTop) = value;

    // move the stackTop just...
    // ... past the last item
    vm.stackTop++;
}

Value pop() {
    // move stack top down
    vm.stackTop--;

    // return item where stack top moved too...
    // ... bc its just past the last item
    return *(vm.stackTop);
}