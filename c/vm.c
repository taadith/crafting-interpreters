#include "common.h"
#include "vm.h"

void initVM(VM* vm) {

}

void freeVM(VM* vm) {

}

static InterpretResult run() {
#define READ_BYTE() (*vm.ip++)

    for(;;) {
        uint8_t instruction;
        switch(instruction = READ_BYTE()) {
            case OP_RETURN: {
                return INTERPRET_OK;
            }
        }
    }

#undef READ_BYTE
}

InterpretResult interpret(VM* vm, Chunk* ch) {
    vm -> chunk = ch;

    // ip pts to the first byte of code in the chunk
    // ip pts to the instruction abt to be executed
    vm -> ip = vm -> chunk -> code;
    return run();
}