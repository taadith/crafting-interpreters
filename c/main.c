#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"

int main(void) {
    VM vm;
    initVM(&vm);

    Chunk chunk;
    initChunk(&chunk);

    writeConstant(&chunk, 1.2, 1);

    writeChunk(&chunk, OP_NEGATE, 1);

    writeChunk(&chunk, OP_RETURN, 1);

    disassembleChunk(&chunk, "negation chunk");

    interpret(&vm, &chunk);

    freeVM(&vm);
    freeChunk(&chunk);

    return 0;
}
