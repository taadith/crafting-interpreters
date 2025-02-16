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

static int simpleInstruction(const char* name, int offset) {
    printf("%s\n", name);

    // skips past opcode
    return offset + 1;
}

// int disassembleInstruction(Chunk* chunk, int offset) {
//     // prints byte offset of the given instruction...
//     // ... telling us where in the chunk the instruction is
//     printf("%04d ", offset);
// 
//     if (offset > 0 &&
//             chunk -> lines[offset] == chunk -> lines[offset - 1]) {
//         printf("    | ");
//     }
//     else {
//         printf("%4d ", chunk -> lines[offset]);
//     }
//     
//     // read the opcode
//     uint8_t instruction = chunk -> code[offset];
//     switch(instruction) {
//         case OP_CONSTANT:
//             return constantInstruction("OP_CONSTANT", chunk, offset);
//         case OP_RETURN:
//             return simpleInstruction("OP_RETURN", offset);
//         default:
//             printf("unknown opcode %d\n", instruction);
//             return offset + 1;
//     }
// }

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
        case OP_RETURN:
            return simpleInstruction("OP_RETURN", offset);
        default:
            printf("unknown opcode %d\n", instruction);
            return offset + 1;
    }
}
