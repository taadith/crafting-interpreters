#include <stdio.h>

#include "common.h"
#include "compiler.h"
#include "scanner.h"

bool compile(Chunk* chunk, const char* src) {
    initScanner(src);

    // "'primes the pump' on the scanner"
    advance();

    // parse a single expression
    expression();

    // check for the sentinel EOF token
    consume(TOKEN_EOF, "expected end of expression");
}
