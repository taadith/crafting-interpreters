#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "rle.h"

void test_chunk_easy(void) {
    Chunk chunk;
    initChunk(&chunk);

    int constant = addConstant(&chunk, 1.2);
    writeChunk(&chunk, OP_CONSTANT, 123);
    writeChunk(&chunk, constant, 123);
    
    writeChunk(&chunk, OP_RETURN, 123);
    
    disassembleChunk(&chunk, "test chunk");
    freeChunk(&chunk);
}

int main(int argc, const char* argv[]) {
    // Chunk testing
    test_chunk_easy();

    return 0;
}
