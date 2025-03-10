#include "common.h"
#include "chunk.h"
#include "debug.h"

void test_writing_easy1(void) {
    Chunk chunk;
    initChunk(&chunk);

    int constant = addConstant(&chunk, 1.2);
    writeChunk(&chunk, OP_CONSTANT, 123);
    writeChunk(&chunk, constant, 123);

    writeChunk(&chunk, OP_RETURN, 124);
    
    disassembleChunk(&chunk, "test-writing-easy1");
    freeChunk(&chunk);
}

void test_writing_easy2(void) {
    Chunk chunk;
    initChunk(&chunk);

    int constant = addConstant(&chunk, 1.2);
    writeChunk(&chunk, OP_CONSTANT, 123);
    writeChunk(&chunk, constant, 123);

    constant = addConstant(&chunk, 1.3);
    writeChunk(&chunk, OP_CONSTANT, 124);
    writeChunk(&chunk, constant, 124);

    writeChunk(&chunk, OP_RETURN, 125);
    
    disassembleChunk(&chunk, "test-writing-easy2");
    freeChunk(&chunk);
}

void test_writing_medium_n(int n) {
    Chunk chunk;
    initChunk(&chunk);

    for(int i = 0; i < n; i++)
        writeConstant(&chunk, (i + 1) * .3, i + 1);

    disassembleChunk(&chunk, "test-writing-medium_n");
    freeChunk(&chunk);
}

int main(void) {
    test_writing_medium_n((1 << 8) | 1);

    return 0;
}
