#include <stdio.h>

#include "common.h"
#include "chunk.h"
#include "rle.h"
#include "debug.h"
#include "vm.h"

void testRunLengthEncoding() {
    printf("starting RunLengthEncoding test...\n");

    RunLengthEncoding rle;
    initRunLengthEncoding(&rle);

    for(int i = 0; i < 16; i++)
        writeRunLengthEncoding(&rle, i / 4);

    printRunLengthEncoding(&rle);

    freeRunLengthEncoding(&rle);
    
    printf("... completed RunLengthEncoding test\n\n");
}

void testVM() {
    printf("started VM test...\n");
    
    initVM();

    Chunk chunk;
    initChunk(&chunk);

    int constant = addConstant(&chunk, 1);
    writeChunk(&chunk, OP_CONSTANT, 1);
    writeChunk(&chunk, constant, 2);

    constant = addConstant(&chunk, 2);
    writeChunk(&chunk, OP_CONSTANT, 2);
    writeChunk(&chunk, constant, 3);

    writeChunk(&chunk, OP_ADD, 4);

    constant = addConstant(&chunk, 3);
    writeChunk(&chunk, OP_CONSTANT, 4);
    writeChunk(&chunk, constant, 5);

    writeChunk(&chunk, OP_DIVIDE, 6);

    writeChunk(&chunk, OP_NEGATE, 7);
    
    writeChunk(&chunk, OP_RETURN, 8);

    // VM springs into action for interpreting...
    // ... a chunk of bytecode
    interpret(&chunk);

    freeVM();

    freeChunk(&chunk);
    
    printf("\n... completed VM test\n");
}

int main(int argc, const char* argv[]) {
    // testRunLengthEncoding();
    testVM();
    return 0;
}