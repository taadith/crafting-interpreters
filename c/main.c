#include <stdio.h>

#include "common.h"
#include "chunk.h"
#include "rle.h"
#include "debug.h"

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

void testChunk() {
    printf("started Chunk test...\n");

    Chunk chunk;
    initChunk(&chunk);

    int constant = addConstant(&chunk, 1.2);
    writeChunk(&chunk, OP_CONSTANT, 123);
    writeChunk(&chunk, constant, 123);

    writeChunk(&chunk, OP_RETURN, 123);

    disassembleChunk(&chunk, "test chunk");

    freeChunk(&chunk);
    
    printf("... completed Chunk test\n");
}

int main(int argc, const char* argv[]) {
    testRunLengthEncoding();
    testChunk();
    return 0;
}