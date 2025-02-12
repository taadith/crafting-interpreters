#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "rle.h"

void test_chunk_simple(void) {
    Chunk chunk;
    initChunk(&chunk);

    int constant = addConstant(&chunk, 1.2);
    writeChunk(&chunk, OP_CONSTANT, 123);
    writeChunk(&chunk, constant, 123);
    
    writeChunk(&chunk, OP_RETURN, 123);
    
    disassembleChunk(&chunk, "test chunk");
    freeChunk(&chunk);
}

void test_rle_simple(void) {
    RunLengthEncoding rle;
    initRunLengthEncoding(&rle);

    writeRunLengthEncoding(&rle, 1);

    freeRunLengthEncoding(&rle);
}

int main(int argc, const char* argv[]) {
    test_chunk_simple();
    test_rle_simple();
    return 0;
}
