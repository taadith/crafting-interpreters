#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "compiler.h"
#include "scanner.h"

typedef struct {
    Token current;
    Token previous;
    bool hadError;

    // are we in panic mode?
    bool panicMode;
} Parser;

Parser parser;

Chunk* compilingChunk;

static Chunk* currentChunk() {
    return compilingChunk;
}

static void errorAt(Token* token, const char* msg) {
    // already in panic mode, so we suppress any...
    // ... other detected errors...
    if (parser.panicMode)
        return;

    // ... otherwise, we are now in panic mode!
    parser.panicMode = true;

    // print where the error occurred
    fprintf(stderr, "[line %d] error", token -> line);

    // if the lexeme is human-readable, show it
    if (token -> type == TOKEN_EOF) {
        fprintf(stderr, " at end");
    } else if (token -> type == TOKEN_ERROR) {
        // do nothing
    } else {
        fprintf(stderr, " at '%.*s'", token -> length, token -> start);
    }

    // print the error message itself
    fprintf(stderr, ": %s\n", msg);

    // set error flag
    parser.hadError = true;
}

static void errorAtCurrent(const char* msg) {
    // pull location out of current token to tell...
    // ... the user where the error occurred and...
    // ... forward it to `errorAt()`
    errorAt(&parser.previous, msg);
}

static void advance() {
    // steps fwd thru the token stream
    parser.previous = parser.current;

    for (;;) {
        parser.current = scanToken();

        // we skip all error tokens
        if (parser.current.type != TOKEN_ERROR)
            break;

        // parser keeps track of error tokens and ...
        // ... eventually reports them
        errorAtCurrent(parser.current.start);
    }
}

// similar to advance and is the foundation to...
// ... most syntax errors in the compiler
static void consume(TokenType type, const char* msg) {
    // validates that the token has an...
    // ... expected type...
    if (parser.current.type == type) {
        advance();
        return;
    }
    // ... otherwise, report an error
    errorAtCurrent(msg);
}

// appends a single byte to the chunk
static void emitByte(uint8_t byte) {
    writeChunk(currentChunk(), byte, parser.previous.line);
}

static void emitBytes(uint8_t byte1, uint8_t byte2) {
    emitByte(byte1);
    emitByte(byte2);
}

static void emitReturn() {
    emitByte(OP_RETURN);
}

static void endCompiler() {
    emitReturn();
}

bool compile(const char* src, Chunk* chunk) {
    initScanner(src);

    // initalizing module variable attached to Chunk of code
    compilingChunk = chunk;

    // initializing parser fields
    parser.hadError = false;
    parser.panicmode = false;

    // primes the scanner
    advance();

    // parse a single expression
    expression();

    // check for sentinel EOF token
    consume(TOKEN_EOF, "expected end of expression");

    endCompiler();

    return !parser.hadError;
}
