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

    for(int i = 1; i < 6; i++) {
        int constant = addConstant(&chunk, i);
        writeChunk(&chunk, OP_CONSTANT, i);
        writeChunk(&chunk, constant, i);
    }
    
    writeChunk(&chunk, OP_RETURN, 6);

    // disassembleChunk(&chunk, "test chunk");

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