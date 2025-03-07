#include <stdio.h>

#include "common.h"
#include "compiler.h"
#include "scanner.h"

void compile(const char* src) {
    Scanner* scanner = NULL;
    initScanner(scanner, src);
    int line = -1;
    for (;;) {

        // scans token and prints it
        Token token = scanToken(scanner);
        if (token.line != line) {
            printf("%4d ", token.line);
            line = token.line;
        }
        else {
            printf("\t| ");
        }

        printf("%2d '%.*s'\n", token.type, token.length, token.start);
        
        // stops at EOF token or an error
        if (token.type == TOKEN_EOF)
            break;
    }
}
