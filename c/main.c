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