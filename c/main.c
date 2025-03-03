#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"

int main(void) {
    VM vm;
    initVM(&vm);

    Chunk chunk;
    initChunk(&chunk);

    writeConstant(&chunk, 1.2, 123);
    writeConstant(&chunk, 3.4, 123);

    // results in 4.6
    writeChunk(&chunk, OP_ADD, 123);

    writeConstant(&chunk, 5.6, 123);

    // results in 23/28 ~= 6/7 ~= .85?
    writeChunk(&chunk, OP_DIVIDE, 123);

    writeChunk(&chunk, OP_RETURN, 123);
    
    disassembleChunk(&chunk, "test chunk");
    
    interpret(&vm, &chunk);

    freeVM(&vm);
    freeChunk(&chunk);

    return 0;
}
