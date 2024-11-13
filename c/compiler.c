#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "compiler.h"
#include "scanner.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

typedef struct {
    Token current;
    Token previous;
    bool hadError;
    bool panicMode;
} Parser;

Parser parser;

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

// simple typedef for a function type that...
// ... takes no args and returns nothing:
// void <fn name>() {...}
typedef void (*ParseFn)();

typedef struct {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

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

// reporting error token at `current`
static void errorAtCurrent(const char* msg) {
    errorAt(&parser.current, msg);
}

static void advance() {
    // takes old `current` token and stashes...
    // ... it in the `previous` field
    parser.previous = parser.current;

    // keep reporting errors until we...
    // ... reach the end or a non-error token
    for(;;) {
        // asks scanner for next token...
        // ... and stores it for later use
        parser.current = scanToken();

        if (parser.current.type != TOKEN_ERROR)
            break;
        errorAtCurrent(parser.current.start);
    }
}

// similar to `advance()` in that it reads the next token...
// ... but it also validates that the token has an expected type
static void consume(TokenType type, const char* msg) {
    if (parser.current.type == type) {
        advance();
        return;
    }

    errorAtCurrent(msg);
}

// appends a single byte, either an opcode...
// ... or an operand to an instruction, to the chunk
static void emitByte(uint8_t byte) {
    writeChunk(currentChunk(), byte, parser.previous.line);
}

// useful for writing an opcode and a one-byte operand
static void emitBytes(uint8_t byte1, uint8_t byte2) {
    emitByte(byte1);
    emitByte(byte2);
}

// inserts an entry into the constant table
static uint8_t makeConstant(Value value) {
    // value's index in the constant tabler
    int constant = addConstant(currentChunk(), value);

    // we can't have more than 256 constants at a time
    if (constant > UINT8_MAX) {
        error("too many constants in one chunk");
        return 0;
    }

    return (uint8_t)constant;
}

// (1) add value to the constant table w/ makeConstant()
// (2) emit an OP_CONSTANT instruction that pushes...
//     ... value onto the stack at runtime
static void emitConstant(Value value) {
    emitBytes(OP_CONSTANT, makeConstant(value));
}

// appends OP_RETURN to chunk
static void emitReturn() {
    emitByte(OP_RETURN);
}

static void endCompiler() {
    emitReturn();

// print out chunk's bytecode...
// ... only if there is no error
#ifdef DEBUG_PRINT_CODE
    if (!parser.hadError)
        disassembleChunk(currentChunk(), "code");
#endif

}

// function annotations
static void expression();
static ParseRule* getRule(TokenType type);
static void parsePrecedence(Precedence precedence);

static void binary() {
    TokenType operatorType = parser.previous.type;
    ParseRule* rule = getRule(operatorType);

    // use `+ 1` bc binary operators are left-associative
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

static void literal() {
    switch(parser.previous.type) {
        case TOKEN_FALSE:
            emitByte(OP_FALSE);
            break;
        case TOKEN_NIL:
            emitByte(OP_NIL);
            break;
        case TOKEN_TRUE:
            emitByte(OP_TRUE);
            break;
        
        // unreachable
        default:
            return;
    }
}

// grouping -> "(" expression ")" ;
// "(" is assumed to have already been consumed
static void grouping() {
    expression();
    consume(TOKEN_RIGHT_PAREN, "expected ')' after expression");
}

static void number() {
    // take lexeme stored in previous and use...
    // ... C STL to convert it to a double value
    double value = strtod(parser.previous.start, NULL);
    
    // generate opcode to load value
    //     - wrap C double in a Value struct before...
    //       ... storing it in the constant table
    emitConstant(NUMBER_VAL(value));
}

static void unary() {
    TokenType operatorType = parser.previous.type;

    // compile the operand before we negate it
    parsePrecedence(PREC_UNARY);    // can only parse >= PREC_UNARY

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

ParseRule rules[] = {
    [TOKEN_LEFT_PAREN]      = {grouping,    NULL,   PREC_NONE},
    [TOKEN_RIGHT_PAREN]     = {NULL,        NULL,   PREC_NONE},
    [TOKEN_LEFT_BRACE]      = {NULL,        NULL,   PREC_NONE}, 
    [TOKEN_RIGHT_BRACE]     = {NULL,        NULL,   PREC_NONE},
    [TOKEN_COMMA]           = {NULL,        NULL,   PREC_NONE},
    [TOKEN_DOT]             = {NULL,        NULL,   PREC_NONE},
    [TOKEN_MINUS]           = {unary,       binary, PREC_TERM},
    [TOKEN_PLUS]            = {NULL,        binary, PREC_TERM},
    [TOKEN_SEMICOLON]       = {NULL,        NULL,   PREC_NONE},
    [TOKEN_SLASH]           = {NULL,        binary, PREC_FACTOR},
    [TOKEN_STAR]            = {NULL,        binary, PREC_FACTOR},
    [TOKEN_BANG]            = {NULL,        NULL,   PREC_NONE},
    [TOKEN_BANG_EQUAL]      = {NULL,        NULL,   PREC_NONE},
    [TOKEN_EQUAL]           = {NULL,        NULL,   PREC_NONE},
    [TOKEN_EQUAL_EQUAL]     = {NULL,        NULL,   PREC_NONE},
    [TOKEN_GREATER]         = {NULL,        NULL,   PREC_NONE},
    [TOKEN_GREATER_EQUAL]   = {NULL,        NULL,   PREC_NONE},
    [TOKEN_LESS]            = {NULL,        NULL,   PREC_NONE},
    [TOKEN_LESS_EQUAL]      = {NULL,        NULL,   PREC_NONE},
    [TOKEN_IDENTIFIER]      = {NULL,        NULL,   PREC_NONE},
    [TOKEN_STRING]          = {NULL,        NULL,   PREC_NONE},
    [TOKEN_NUMBER]          = {number,      NULL,   PREC_NONE},
    [TOKEN_AND]             = {NULL,        NULL,   PREC_NONE},
    [TOKEN_CLASS]           = {NULL,        NULL,   PREC_NONE},
    [TOKEN_ELSE]            = {NULL,        NULL,   PREC_NONE},
    [TOKEN_FALSE]           = {literal,     NULL,   PREC_NONE},
    [TOKEN_FOR]             = {NULL,        NULL,   PREC_NONE},
    [TOKEN_FUN]             = {NULL,        NULL,   PREC_NONE},
    [TOKEN_IF]              = {NULL,        NULL,   PREC_NONE},
    [TOKEN_NIL]             = {literal,     NULL,   PREC_NONE},
    [TOKEN_OR]              = {NULL,        NULL,   PREC_NONE},
    [TOKEN_PRINT]           = {NULL,        NULL,   PREC_NONE},
    [TOKEN_RETURN]          = {NULL,        NULL,   PREC_NONE},
    [TOKEN_SUPER]           = {NULL,        NULL,   PREC_NONE},
    [TOKEN_THIS]            = {NULL,        NULL,   PREC_NONE},
    [TOKEN_TRUE]            = {literal,     NULL,   PREC_NONE},
    [TOKEN_VAR]             = {NULL,        NULL,   PREC_NONE},
    [TOKEN_WHILE]           = {NULL,        NULL,   PREC_NONE},
    [TOKEN_ERROR]           = {NULL,        NULL,   PREC_NONE},
    [TOKEN_EOF]             = {NULL,        NULL,   PREC_NONE},
};

// starts at the current token and parses any...
// ... expression at the given precedence lvl or higher
static void parsePrecedence(Precedence precedence) {
    // read the next token
    advance();

    // look up a prefix parser for the previous token
    ParseFn prefixRule = getRule(parser.previous.type) -> prefix;
    
    // no prefix rule means that...
    // ... the token must be a syntax error
    if (prefixRule == NULL) {
        error("expected expression");
        return;
    }

    prefixRule();

    // looking for an infix parser for the next token
    while(precedence <= (getRule(parser.current.type) -> precedence)) {
        advance();
        ParseFn infixRule = getRule(parser.previous.type) -> infix;
        infixRule();
    }
}

static ParseRule* getRule(TokenType type) {
    return &rules[type];
}

static void expression() {
    // simply parse the lowest precedence lvl
    parsePrecedence(PREC_ASSIGNMENT);
}

// returns whether or not compilation succeeded
// compiler writes opcode into `chunk`
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