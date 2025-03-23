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

    // are we in panic mode?
    bool panicMode;
} Parser;

// Lox's precedence lvls in order from lowest...
// ... to highest
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

// ParseFn is a function type that takes...
// ... no arguments and returns nothing
typedef void (*ParseFn)(void);

// reps a single row in the parser table
typedef struct {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

Parser parser;

Chunk* compilingChunk;

static Chunk* currentChunk(void) {
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

static void error(const char* msg) {
    errorAt(&parser.previous, msg);
}

static void errorAtCurrent(const char* msg) {
    // pull location out of current token to tell...
    // ... the user where the error occurred and...
    // ... forward it to `errorAt()`
    errorAt(&parser.previous, msg);
}

static void advance(void) {
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

static void emitReturn(void) {
    emitByte(OP_RETURN);
}

// TODO: be able to return "uint24_t"
static uint8_t makeConstant(Value value) {
    // adds the given value to the end of the chunk's...
    // ... constant table and returns its index
    int constant_index = addConstant(currentChunk(), value);

    // TODO: change to "UINT24_MAX"
    // handles bounds checking for constant index...
    // ... can only handle up to 256 constants in a chunk
    if (constant_index > UINT8_MAX) {
        error("too many constants in one chunk");
        return 0;
    }

    return (uint8_t)constant_index;
}

static void emitConstant(Value value) {
    emitBytes(OP_CONSTANT, makeConstant(value));
}

static void endCompiler(void) {
    emitReturn();

    #ifdef DEBUG_PRINT_CODE
    if (!parser.hadError)
        disassembleChunk(currentChunk(), "code");
    #endif
}

static void expression(void);
static ParseRule* getRule(TokenType type);
static void parsePrecedence(Precedence precedence);

static void binary(void) {
    // grabbing the precedence of the operator...
    // ... to get the rest of the right operand
    TokenType operatorType = parser.previous.type;
    ParseRule* rule = getRule(operatorType);
    parsePrecedence((Precedence)(rule -> precedence + 1));

    // emites the bytecode instruction that...
    // ... performs the binary operation
    switch (operatorType) {
        case TOKEN_BANG_EQUAL:
            emitBytes(OP_EQUAL, OP_NOT);
            break;
        case TOKEN_EQUAL_EQUAL:
            emitByte(OP_EQUAL);
            break;
        case TOKEN_GREATER:
            emitByte(OP_GREATER);
            break;
        case TOKEN_GREATER_EQUAL:
            emitBytes(OP_LESS, OP_NOT);
            break;
        case TOKEN_LESS:
            emitByte(OP_LESS);
            break;
        case TOKEN_LESS_EQUAL:
            emitBytes(OP_GREATER, OP_NOT);
            break;
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

        // in theory, unreachable
        default:
            return;
    }
}

static void literal(void) {
    switch (parser.previous.type) {
        case TOKEN_FALSE:
            emitByte(OP_FALSE);
            break;
        case TOKEN_NIL:
            emitByte(OP_NIL);
            break;
        case TOKEN_TRUE:
            emitByte(OP_TRUE);
            break;

        // in theory, unreachable code
        default:
            return;
    }
}

// assume the initial '(' has been consumed
static void grouping(void) {
    // recursively call back into `expression()`...
    // ... to compile the expr between the "()"
    expression();

    // parse the closing ')'
    consume(TOKEN_RIGHT_PAREN, "expected ')' after expression");
}

static void number(void) {
    // take number literal and use C std library...
    // ... to convert it a double value
    double value = strtod(parser.previous.start, NULL);

    // generate code to load the value...
    // ... and wrap in a Value before storing...
    // ... it in the constant table
    emitConstant(NUMBER_VAL(value));
}

static void string(void) {
    // takes string's chars directly from the lexeme...
    // ... `+ 1` and `- 2` serve to trim the leading....
    // ... and trailing quotation marks
    emitConstant(OBJ_VAL(copyString(parser.previous.start + 1,
                                    parser.previous.length - 2)));
}

static void unary(void) {
    TokenType operatorType = parser.previous.type;

    // compile the operand
    parsePrecedence(PREC_UNARY);

    // emit the operator instruction
    switch (operatorType) {
        case TOKEN_BANG:
            emitByte(OP_NOT);
            break;

        case TOKEN_MINUS:
            emitByte(OP_NEGATE);
            break;

        // in theory, unreachable
        default:
            return;
    }
}

// the infamous parsing table
ParseRule rules[] = {
    [TOKEN_LEFT_PAREN]    = {grouping, NULL,   PREC_NONE},
    [TOKEN_RIGHT_PAREN]   = {NULL,     NULL,   PREC_NONE},
    [TOKEN_LEFT_BRACE]    = {NULL,     NULL,   PREC_NONE}, 
    [TOKEN_RIGHT_BRACE]   = {NULL,     NULL,   PREC_NONE},
    [TOKEN_COMMA]         = {NULL,     NULL,   PREC_NONE},
    [TOKEN_DOT]           = {NULL,     NULL,   PREC_NONE},
    [TOKEN_MINUS]         = {unary,    binary, PREC_TERM},
    [TOKEN_PLUS]          = {NULL,     binary, PREC_TERM},
    [TOKEN_SEMICOLON]     = {NULL,     NULL,   PREC_NONE},
    [TOKEN_SLASH]         = {NULL,     binary, PREC_FACTOR},
    [TOKEN_STAR]          = {NULL,     binary, PREC_FACTOR},
    [TOKEN_BANG]          = {unary,    NULL,   PREC_NONE},
    [TOKEN_BANG_EQUAL]    = {NULL,     binary, PREC_EQUALITY},
    [TOKEN_EQUAL]         = {NULL,     NULL,   PREC_NONE},
    [TOKEN_EQUAL_EQUAL]   = {NULL,     binary, PREC_EQUALITY},
    [TOKEN_GREATER]       = {NULL,     binary, PREC_COMPARISON},
    [TOKEN_GREATER_EQUAL] = {NULL,     binary, PREC_COMPARISON},
    [TOKEN_LESS]          = {NULL,     binary, PREC_COMPARISON},
    [TOKEN_LESS_EQUAL]    = {NULL,     binary, PREC_COMPARISON},
    [TOKEN_IDENTIFIER]    = {NULL,     NULL,   PREC_NONE},
    [TOKEN_STRING]        = {string,   NULL,   PREC_NONE},
    [TOKEN_NUMBER]        = {number,   NULL,   PREC_NONE},
    [TOKEN_AND]           = {NULL,     NULL,   PREC_NONE},
    [TOKEN_CLASS]         = {NULL,     NULL,   PREC_NONE},
    [TOKEN_ELSE]          = {NULL,     NULL,   PREC_NONE},
    [TOKEN_FALSE]         = {literal,  NULL,   PREC_NONE},
    [TOKEN_FOR]           = {NULL,     NULL,   PREC_NONE},
    [TOKEN_FUN]           = {NULL,     NULL,   PREC_NONE},
    [TOKEN_IF]            = {NULL,     NULL,   PREC_NONE},
    [TOKEN_NIL]           = {literal,  NULL,   PREC_NONE},
    [TOKEN_OR]            = {NULL,     NULL,   PREC_NONE},
    [TOKEN_PRINT]         = {NULL,     NULL,   PREC_NONE},
    [TOKEN_RETURN]        = {NULL,     NULL,   PREC_NONE},
    [TOKEN_SUPER]         = {NULL,     NULL,   PREC_NONE},
    [TOKEN_THIS]          = {NULL,     NULL,   PREC_NONE},
    [TOKEN_TRUE]          = {literal,  NULL,   PREC_NONE},
    [TOKEN_VAR]           = {NULL,     NULL,   PREC_NONE},
    [TOKEN_WHILE]         = {NULL,     NULL,   PREC_NONE},
    [TOKEN_ERROR]         = {NULL,     NULL,   PREC_NONE},
    [TOKEN_EOF]           = {NULL,     NULL,   PREC_NONE},
};

// parsers any expression at the given precedence...
// ... lvl or higher
static void parsePrecedence(Precedence precedence) {
    // read the next token
    advance();

    // look up the corresponding ParseRule
    ParseFn prefixRule = getRule(parser.previous.type) -> prefix;

    // no prefix parser means the token must be...
    // ... a syntax error
    if (prefixRule == NULL) {
        error("expected expression");
        return;
    }

    prefixRule();

    // look for an infix parser for the next token
    while (precedence <= getRule(parser.current.type) -> precedence) {
        advance();
        ParseFn infixRule = getRule(parser.previous.type) -> infix;
        infixRule();
    }
}

// returns the rule at the given index
static ParseRule* getRule(TokenType type) {
    return &rules[type];
}

static void expression(void) {
    parsePrecedence(PREC_ASSIGNMENT);
}

bool compile(const char* src, Chunk* chunk) {
    initScanner(src);

    // initalizing module variable attached to Chunk of code
    compilingChunk = chunk;

    // initializing parser fields
    parser.hadError = false;
    parser.panicMode = false;

    // primes the scanner
    advance();

    // parse a single expression
    expression();

    // check for sentinel EOF token
    consume(TOKEN_EOF, "expected end of expression");

    endCompiler();

    return !parser.hadError;
}
