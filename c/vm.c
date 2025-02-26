#include <stdio.h>

#include "common.h"
#include "debug.h"
#include "vm.h"

// TODO: replace static VM with dynamic VM!
// VM vm;

// initializes a VM
void initVM(VM* vm) {

}

// frees a VM
void freeVM(VM* vm) {

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

    for(;;) {
        // diagnostic logging for VM...
        // ... disassembling instructions
        #ifdef DEBUG_TRACE_EXECUTION
        disassembleInstructionWithRLE(vm -> chunk,
                                      (int) (vm -> ip - vm -> chunk -> code
        ));
        #endif

        uint8_t instruction;
        switch (instruction = READ_BYTE()) {
            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                printValue(constant);
                printf("\n");
                break;
            }
            case OP_RETURN: {
                return INTERPRET_OK;
            }
        }
    }

    #undef READ_BYTE
    #undef READ_CONSTANT
}

InterpretResult interpret(VM* vm, Chunk* chunk) {
    vm -> chunk = chunk;
    vm -> ip = (vm -> chunk) -> code;
    return run(vm);
}
