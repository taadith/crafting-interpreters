#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "common.h"
#include "chunk.h"
#include "rle.h"
#include "debug.h"
#include "vm.h"

static void repl() {
    char line[1024];
    for(;;) {
        printf("> ");

        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }

        interpret(line);
    }
}

static char* readFile(const char* path) {
    FILE* file = fopen(path, "rb");

    // failing to open the file
    if (file == NULL) {
        fprintf(stderr, "couldn't open file \"%s\"\n", path);
        exit(74);
    }

    // go to very end
    fseek(file, 0L, SEEK_END);

    // see how many bytes we are away from the start
    size_t fileSize = ftell(file);

    // go back to the beginning
    rewind(file);

    // allocate a string of the file size
    char* buffer = (char*)malloc(fileSize + 1);

    // can't allocate enough memory for Lox script
    if (buffer == NULL) {
        fprintf(stderr, "not enough memory to read \"%s\"", path);
        exit(74);
    }

    // read the whole file in a single batch
    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);

    // the read failed
    if (bytesRead < fileSize) {
        fprintf(stderr, "couldn't read file \"%s\"", path);
        exit(74);
    }

    // the null byte at the end
    buffer[bytesRead] = '\0';

    fclose(file);
    return buffer;
}

static void runFile(const char* path) {
    char* src = readFile(path);
    InterpretResult result = interpret(src);
    free(src);

    if (result == INTERPRET_COMPILE_ERROR)
        exit(65);
    if (result == INTERPRET_RUNTIME_ERROR)
        exit(70);
}

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
    initVM();

    if (argc == 1)
        repl();
    else if (argc == 2)
        runFile(argv[1]);
    else {
        fprintf(stderr, "usage: clox [path]\n");
        exit(64);
    }

    freeVM();
    return 0;
}