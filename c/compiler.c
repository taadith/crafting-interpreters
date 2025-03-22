#include <stdio.h>

#include "common.h"
#include "compiler.h"
#include "scanner.h"

bool compile(const char* src, Chunk* chunk) {
    initScanner(src);

    // primes the scanner
    advance();

    // parse a single expression
    expression();

    // check for sentinel EOF token
    consume(TOKEN_EOF, "expected end of expression");
}
