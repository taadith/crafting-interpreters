#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"

static void repl(void) {
    char line[1024];
    for (;;) {
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
    if (file == NULL) {
        fprintf(stderr, "couldn't open file \"%s\"\n", path);

        // generic input/output failure
        exit(74);
    }

    // go to end of file
    fseek(file, 0L, SEEK_END);

    // tells us what byte of file we are at...
    // ... obviously the last byte
    size_t fileSize = ftell(file);

    // go back to the start of the file
    rewind(file);

    char* buffer = (char*) malloc(fileSize + 1);
    if (buffer == NULL) {
        fprintf(stderr, "not enough memory to read \"%s\"\n", path);
        exit(74);
    }

    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    if (bytesRead < fileSize) {
        fprintf(stderr, "couldn't read file \"%s\"\n", path);
        exit(74);
    }

    buffer[bytesRead] = '\0';

    fclose(file);
    return buffer;
}

static void runFile(const char* path) {
    // read the file and exec the resulting...
    // ... string of Lox src code
    char* src = readFile(path);
    InterpretResult result = interpret(src);
    free(src);

    // data formatted incorrectly/unexpectedly
    if (result == INTERPRET_COMPILE_ERROR)
        exit(65);

    // unhandled error in S/W or logic
    if (result == INTERPRET_RUNTIME_ERROR)
        exit(70);
}

int main(int argc, const char* argv[]) {
    VM* vm = NULL;
    initVM(vm);

    if (argc == 1) {
        repl();
    }
    else if (argc == 2) {
        runFile(argv[1]);
    }
    else {
        fprintf(stderr, "usage: clox [path]\n");

        // command-line usage error
        exit(64);
    }

    freeVM(vm);
    return 0;
}
