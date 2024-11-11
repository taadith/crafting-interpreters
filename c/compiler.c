#include <stdio.h>

#include "common.h"
#include "compiler.h"
#include "scanner.h"

void compile(const char* src) {
    initScanner(src);
    int line = -1;

    // loops indefinitely
    for(;;) {
        // scans one token and then prints it
        Token token = scanToken();
        if (token.line != line) {
            printf("%4d ", token.line);
            line = token.line;
        }
        else
            printf("   | ");
        printf("%2d '%.*s'\n", token.type, token.length, token.start);

        // stops on the "EOF" token or an error token
        if (token.type == TOKEN_EOF)
            break;
    }
}