#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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

void testTimeOfNegate(int n) {
    // want it to be more than zero!
    if (n <= 0)
        exit(1);
    
    clock_t start;
    clock_t end;
    clock_t total_time_taken = 0;
    for(int i = 0; i < n; i++) {
        start = clock();
        initVM();

        Chunk chunk;
        initChunk(&chunk);

        int constant = addConstant(&chunk, -100);
        writeChunk(&chunk, OP_CONSTANT, 1);
        writeChunk(&chunk, constant, 1);

        writeChunk(&chunk, OP_NEGATE, 1);
        
        writeChunk(&chunk, OP_RETURN, 1);

        // VM springs into action for interpreting...
        // ... a chunk of bytecode
        interpret(&chunk);

        freeVM();

        freeChunk(&chunk);

        end = clock();
        total_time_taken += ((double)(end - start)) / CLOCKS_PER_SEC;
    }

    long double avg_time_taken = total_time_taken / n;
    printf("average time over %d trial(s) to negate one constant on the stack: %Lf", n, avg_time_taken);
}

int main(int argc, const char* argv[]) {
    testVM();
    return 0;
}