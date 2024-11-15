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
typedef void (*ParseFn)(void);

typedef struct {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

Chunk* compilingChunk;

static Chunk* currentChunk(void) {
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

static void advance(void) {
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

// returns true if the current...
// ... token has the given type
static bool check(TokenType type) {
    return parser.current.type == type;
}

// consumes the current token and returns true...
// ... if the current token has the given `type`
static bool match(TokenType type) {
    // current token doesn't match the...
    // ... given `type` so return false
    if (!check(type))
        return false;

    // current token has the given `type`...
    // ... so we consume the token and return true
    advance();
    return true;
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
static void emitReturn(void) {
    emitByte(OP_RETURN);
}

static void endCompiler(void) {
    emitReturn();

// print out chunk's bytecode...
// ... only if there is no error
#ifdef DEBUG_PRINT_CODE
    if (!parser.hadError)
        disassembleChunk(currentChunk(), "code");
#endif

}

// function annotations
static void expression(void);
static void statement(void);
static void declaration(void);
static ParseRule* getRule(TokenType type);
static void parsePrecedence(Precedence precedence);

static void binary(void) {
    TokenType operatorType = parser.previous.type;
    ParseRule* rule = getRule(operatorType);

    // use `+ 1` bc binary operators are left-associative
    parsePrecedence((Precedence) (rule -> precedence + 1));

    switch(operatorType) {
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

        // unreachable code
        default:
            return;
    }
}

static void literal(void) {
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
static void grouping(void) {
    expression();
    consume(TOKEN_RIGHT_PAREN, "expected ')' after expression");
}

static void number(void) {
    // take lexeme stored in previous and use...
    // ... C STL to convert it to a double value
    double value = strtod(parser.previous.start, NULL);
    
    // generate opcode to load value
    //     - wrap C double in a Value struct before...
    //       ... storing it in the constant table
    emitConstant(NUMBER_VAL(value));
}

// takes string's chars directly from the lexeme and...
// ... creates a string obj which is wrapped in a Value
static void string(void) {
    // +1 and -2 trims leading and trailing quotation marks
    emitConstant(OBJ_VAL(copyString(parser.previous.start + 1, parser.previous.length - 2)));
}

static void namedVariable(Token name) {
    // takes given identifier token and adds its lexeme to...
    // ... the chunk's constant table as a string
    uint8_t arg = identifierConstant(&name);
    emitBytes(OP_GET_GLOBAL, arg);
}

static void variable(void) {
    namedVariable(parser.previous);
}

static void unary(void) {
    TokenType operatorType = parser.previous.type;

    // compile the operand before we negate it
    parsePrecedence(PREC_UNARY);    // can only parse >= PREC_UNARY

    // emit the operator instruction
    switch(operatorType) {
        case TOKEN_BANG:
            emitByte(OP_NOT);
            break;
        
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
    [TOKEN_BANG]            = {unary,       NULL,   PREC_NONE},
    [TOKEN_BANG_EQUAL]      = {NULL,        binary, PREC_EQUALITY},
    [TOKEN_EQUAL]           = {NULL,        NULL,   PREC_NONE},
    [TOKEN_EQUAL_EQUAL]     = {NULL,        binary, PREC_EQUALITY},
    [TOKEN_GREATER]         = {NULL,        binary, PREC_COMPARISON},
    [TOKEN_GREATER_EQUAL]   = {NULL,        binary, PREC_COMPARISON},
    [TOKEN_LESS]            = {NULL,        binary, PREC_COMPARISON},
    [TOKEN_LESS_EQUAL]      = {NULL,        binary, PREC_COMPARISON},
    [TOKEN_IDENTIFIER]      = {variable,    NULL,   PREC_NONE},
    [TOKEN_STRING]          = {string,      NULL,   PREC_NONE},
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

// takes the given token and adds its lexeme...
// ... to the chunk's constant table as a string...
// ... and then returns the index of the constant...
// ... in the constant table
static uint8_t identifierConstant(Token* name) {
    return makeConstant(OBJ_VAL(copyString(name -> start,
                                           name -> length)));
}

// requires next token to be an identifier
static uint8_t parseVariable(const char* errorMsg) {
    consume(TOKEN_IDENTIFIER, errorMsg);

    // index from constant table
    return identifierConstant(&parser.previous);
}

// outputs the bytecode instruction that defines...
// ... the new variable and stores its initial value
static void defineVariable(uint8_t global) {
    emitBytes(OP_DEFINE_GLOBAL, global);
}

static ParseRule* getRule(TokenType type) {
    return &rules[type];
}

static void expression(void) {
    // simply parse the lowest precedence lvl
    parsePrecedence(PREC_ASSIGNMENT);
}

static void varDeclaration() {
    // the variable name
    uint8_t global = parseVariable("expected variable name");

    // initializing the variable
    if (match(TOKEN_EQUAL))
        expression();
    else
        emitByte(OP_NIL);
    
    consume(TOKEN_SEMICOLON, "expected ';' after variable declaration");

    defineVariable(global);
}

static void expressionStatement(void) {
    expression();
    consume(TOKEN_SEMICOLON, "expected ';' after expression");
    emitByte(OP_POP);
}

static void printStatement(void) {
    expression();
    consume(TOKEN_SEMICOLON, "expected ';' after value");
    emitByte(OP_PRINT);
}

// skip tokens indiscrimately until we reach something...
// ... that looks like a statement boundary
static void synchronize(void) {
    parser.panicMode = false;

    while(parser.current.type != TOKEN_EOF) {
        if (parser.previous.type == TOKEN_SEMICOLON)
            return;
        
        switch(parser.current.type) {
            case TOKEN_CLASS:
            case TOKEN_FUN:
            case TOKEN_VAR:
            case TOKEN_IF:
            case TOKEN_WHILE:
            case TOKEN_PRINT:
            case TOKEN_RETURN:
                return;
            
            // do nothing
            default:
                ;
        }

        advance();
    }
}

static void declaration(void) {
    if (match(TOKEN_VAR))
        varDeclaration();
    else
        statement();

    // hitting a compile error...
    // ... causes a panic mode
    if (parser.panicMode)
        synchronize();
}

static void statement(void) {
    if (match(TOKEN_PRINT))
        printStatement();
    else
        expressionStatement();
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

    while(!match(TOKEN_EOF))
        declaration();

    endCompiler();

    // returns false if an error occured
    return !parser.hadError;
}
