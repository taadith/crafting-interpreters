#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "compiler.h"
#include "scanner.h"

typedef struct {
    Token current;
    Token previous;
    bool hadError;
    bool panicMode;
} Parser;

// all of Lox's precedence lvls...
// ... from lowest to highest
typedef enum {
    PREC_NONE,
    PREC_ASSIGNMENT,    // =
    PREC_OR,            // or
    PREC_AND,           // and
    PREC_EQUALITY,      // == !=
    PREC_COMPARISON,    // < > <= >=
    PREC_TERM,          // + -
    PREC_FACTOR,        // * /
    PREC_UNARY,         // ! -
    PREC_CALL,          // . ()
    PREC_PRIMARY
} Precedence;

Parser parser;
Chunk* compilingChunk;

static Chunk* currentChunk() {
    return compilingChunk;
}

static void errorAt(Token* token, const char* msg) {
    // exit out of function unless we...
    // ... haven't entered panic mode yet
    if (parser.panicMode)
        return;
    parser.panicMode = true;

    // print where error occured
    fprintf(stderr, "[line %d] Error", token -> line);

    // what kind of token is it?
    if (token -> type == TOKEN_EOF)
        fprintf(stderr, " at end");
    else if (token -> type == TOKEN_ERROR) {}
    else
        fprintf(stderr, " at '%.*s'", token -> length, token -> start);
    
    // print the error msg itself
    fprintf(stderr, ": %s\n", msg);

    // set the `hadError` flag
    parser.hadError = true;
}

// reporting error at previous token
static void error(const char* msg) {
    errorAt(&parser.previous, msg);
}

// reporting error at current token
static void errorAtCurrent(const char* msg) {
    errorAt(&parser.current, msg);
}

static void advance() {
    parser.previous = parser.current;

    // keep reporting errors until we...
    // ... reach the end or a non-error token
    for(;;) {
        // here's where the scanner...
        // ... emits a token to the parser
        parser.current = scanToken();

        if (parser.current.type != TOKEN_ERROR)
            break;
        errorAtCurrent(parser.current.start);
    }
}

static void consume(TokenType type, const char* msg) {
    if (parser.current.type == type) {
        advance();
        return;
    }

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

// inserts an entry into the constant table
static uint8_t makeConstant(Value value) {
    // value's index in the constant tabler
    int constant = addConstant(currentChunk(), value);

    // checks that value's index isn't greater than UINT8_MAX
    if (constant > UINT8_MAX) {
        error("too many constants in one chunk");
        return 0;
    }

    return (uint8_t)constant;
}

static void emitConstant(Value value) {
    emitBytes(OP_CONSTANT, makeConstant(value));
}

static void emitReturn() {
    emitByte(OP_RETURN);
}

static void endCompiler() {
    emitReturn();
}

static void binary() {
    TokenType operatorType = parser.previous.type;
    ParseRule* rule = getRule(operatorType);
    parsePrecedence((Precedence) (rule -> precedence + 1));

    switch(operatorType) {
        case TOKEN_PLUS:
            emitByte(OP_ADD);
            break;
        case TOKEN_MINUS:
            emitByte(OP_SUBTRACT);
            break;
        case TOKEN_STAR:
            emitByte(OP_MULTIPLY);
            break;
        case TOKEN_SLASH:
            emitByte(OP_DIVIDE);
            break;

        // unreachable code
        default:
            return;
    }
}

static void grouping() {
    expression();
    consume(TOKEN_RIGHT_PAREN, "expected ')' after expression");
}

static void number() {
    double value = strtod(parser.previous.start, NULL);
    emitConstant(value);
}

static void unary() {
    TokenType operatorType = parser.previous.type;

    // compile the operand
    parsePrecedence(PREC_UNARY);

    // emit the operator instruction
    switch(operatorType) {
        case TOKEN_MINUS:
            emitByte(OP_NEGATE);
            break;
        
        // unreachable
        default:
            return;
    }
}

static void parsePrecedence(Precedence precedence) {
    // what goes here?
}

static void expression() {
    parsePrecedence(PREC_ASSIGNMENT);
}

bool compile(const char* src, Chunk* chunk) {
    initScanner(src);
    compilingChunk = chunk;

    parser.hadError = false;
    parser.panicMode = false;

    // "primes the pump" on the scanner
    advance();

    // parse a single expression
    expression();

    // checking for sentinel EOF token
    consume(TOKEN_EOF, "expected end of expression");

    endCompiler();

    // returns false if an error occured
    return !parser.hadError;
}