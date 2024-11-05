#include <stdio.h>

#include "common.h"
#include "debug.h"
#include "vm.h"

VM vm;

void initVM() {

}

void freeVM() {

}

static InterpretResult run() {
// reads current byte pointed at by ip...
// ... and then advances the ip
#define READ_BYTE() (*vm.ip++)

// reads the next byte from the bytecode...
// ... treats the resulting # as an index...
// ... and looks up the corresponding Value in the chunk's constant table
#define READ_CONSTANT() (vm.chunk -> constants.values[READ_BYTE()])

    for(;;) {

// dynamic debugging if flag is defined
#ifdef DEBUG_TRACE_EXECUTION
        disassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk -> code));
#endif

        // first byte of any instruction is the opcode
        uint8_t instruction;
        switch(instruction = READ_BYTE()) {
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

InterpretResult interpret(Chunk* ch) {
    vm.chunk = ch;

    // ip pts to the first byte of code in the chunk
    // ip pts to the instruction abt to be executed
    vm.ip = vm.chunk -> code;
    return run();
}