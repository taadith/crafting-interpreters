#include <stdio.h>

#include "debug.h"

void disassembleChunk(Chunk* chunk, const char* name) {
    // print header of chunk
    printf("==%s==\n", name);
    
    for(int offset = 0; offset < chunk -> count;) {
        // dissassembleInstruction increments offset
        offset = disassembleInstruction(chunk, offset);
    }
}

static int simpleInstruction(const char* name, int offset) {
    printf("%s\n", name);
    return offset + 1;
}

int disassembleInstruction(Chunk* chunk, int offset) {
    // prints byte offset of the given instruction...
    // ... telling us where in the chunk the instruction is
    printf("%04d ", offset);
    
    // read the opcode
    uint8_t instruction = chunk -> code[offset];
    switch(instruction) {
        case OP_RETURN:
            return simpleInstruction("OP_RETURN", offset);
        default:
            printf("unknown opcode %d\n", instruction);
            return offset + 1;
    }
}
