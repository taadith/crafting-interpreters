#include <stdio.h>

#include "debug.h"
#include "value.h"

void disassembleChunk(Chunk* chunk, const char* name) {
    // print header of chunk
    printf("==%s==\n", name);
    
    // disassemble each instruction
    for(int offset = 0; offset < chunk -> count;) {
        offset = disassembleInstructionWithRLE(chunk, offset);
    }
}

static int constantInstruction(const char* name, Chunk* chunk,
                               int offset) {
    // grab the constant index
    uint8_t constant_index = chunk -> code[offset + 1];

    // print out the name of the opcode and constant index
    printf("%-16s %4d '", name, constant_index);

    // look up the actual constant value
    printValue(chunk -> constants.values[constant_index]);

    printf("'\n");
    
    // skips past opcode and constant index
    return offset + 2;
}

static int constantLongInstruction(const char* name, Chunk* chunk,
                               int offset) {
    // grab the constant index
    uint8_t first_byte_constant_index = chunk -> code[offset + 1];
    uint8_t second_byte_constant_index = chunk -> code[offset + 2];
    uint8_t third_byte_constant_index = chunk -> code[offset + 3];

    int constant_index = (first_byte_constant_index << 16) |
                         (second_byte_constant_index << 8) |
                         (third_byte_constant_index);

    // print out the name of the opcode and constant index
    printf("%-16s %4d '", name, constant_index);

    // look up the actual constant value
    printValue(chunk -> constants.values[constant_index]);

    printf("'\n");

    // skips past opcode and three bytes
    return offset + 4;
}

static int simpleInstruction(const char* name, int offset) {
    printf("%s\n", name);

    // skips past opcode
    return offset + 1;
}

int disassembleInstructionWithRLE(Chunk* chunk, int offset) {
    // prints byte offset of the given instruction...
    // ... telling us where in the chunk the instruction is
    printf("%04d ", offset);

    if (offset > 0 &&
            getLine(chunk, offset) == getLine(chunk, offset - 1)) {
        printf("    | ");
    }
    else {
        printf("%4d ", getLine(chunk, offset));
    }
    
    // read the opcode
    uint8_t instruction = chunk -> code[offset];
    switch(instruction) {
        case OP_CONSTANT:
            return constantInstruction("OP_CONSTANT", chunk, offset);
        case OP_CONSTANT_LONG:
            return constantLongInstruction("OP_CONSTANT_LONG", chunk, offset);
        case OP_NIL:
            return simpleInstruction("OP_NIL", offset);
        case OP_TRUE:
            return simpleInstruction("OP_TRUE", offset);
        case OP_FALSE:
            return simpleInstruction("OP_FALSE", offset);
        case OP_EQUAL:
            return simpleInstruction("OP_EQUAL", offset);
        case OP_GREATER:
            return simpleInstruction("OP_GREATER", offset);
        case OP_LESS:
            return simpleInstruction("OP_LESS", offset);
        case OP_ADD:
            return simpleInstruction("OP_ADD", offset);
        case OP_SUBTRACT:
            return simpleInstruction("OP_SUBTRACT", offset);
        case OP_MULTIPLY:
            return simpleInstruction("OP_MULTIPLY", offset);
        case OP_DIVIDE:
            return simpleInstruction("OP_DIVIDE", offset);
        case OP_NOT:
            return simpleInstruction("OP_NOT", offset);
        case OP_NEGATE:
            return simpleInstruction("OP_NEGATE", offset);
        case OP_RETURN:
            return simpleInstruction("OP_RETURN", offset);
        default:
            printf("unknown opcode %d\n", instruction);
            return offset + 1;
    }
}
