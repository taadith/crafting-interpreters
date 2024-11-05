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

    int constant = addConstant(&chunk, 1.2);
    writeChunk(&chunk, OP_CONSTANT, 123);
    writeChunk(&chunk, constant, 123);

    writeChunk(&chunk, OP_RETURN, 123);

    // disassembleChunk(&chunk, "test chunk");

    // VM springs into action for interpreting...
    // ... a chunk of bytecode
    interpret(&chunk);

    freeVM();

    freeChunk(&chunk);
    
    printf("... completed VM test\n");
}

int main(int argc, const char* argv[]) {
    testRunLengthEncoding();
    testVM();
    return 0;
}