#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
// ... takes 1 bool arg and returns nothing:
// void <fn name>(bool canAssign) {...}
typedef void (*ParseFn)(bool canAssign);

typedef struct {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

typedef struct {
    // the name of the variable
    Token name;

    // records the scope depth of the block...
    // ... where the local variable was declared
    int depth;
} Local;

// how we keep track of what type of code...
// ... we are looking at!
typedef enum {
    TYPE_FUNCTION,
    TYPE_SCRIPT,
} FunctionType;

typedef struct {
    // top-lvl code is placed in an...
    // ... automatically defined function
    ObjFunction* function;

    // tells compiler whether we are looking...
    // ... at top-lvl code or a function
    FunctionType type;

    // simple flat array of all locals that are in...
    // ... scope during each point in the compilation proc
    Local locals[UINT8_COUNT];
    
    // tracks how many locals are in scope basically how...
    // ... many of the slots in `locals` array are in use
    int localCount;
    
    // # of blocks surrounding the current code being compiled
    int scopeDepth;
} Compiler;

Compiler* current = NULL;

static Chunk* currentChunk(void) {
    return &current -> function -> chunk;
}

// Chunk* compilingChunk;
//
// static Chunk* currentChunk(void) {
    // return compilingChunk;
// }

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
    // value's index in the constant table
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

// emits a bytecode instruction and writes a...
// placeholder instruction for the jump offset
static int emitJump(uint8_t instruction) {
    // emits the bytecode instruction
    emitByte(instruction);

    #ifdef DEBUG_CONTROL_FLOW
        printf("we should be returning %d", currentChunk() -> count);
        printf("emitted instruction\n");
    #endif

    // two bytes for the placeholder which...
    // ... later becomes the jump offset operand
    emitBytes(0xff, 0xff);

    #ifdef DEBUG_CONTROL_FLOW
        printf("emitted placeholder for jump offset\n");
        printf("returning %d", currentChunk() -> count - 2);
    #endif

    // returns the offset of the emitted instruction in the chunk
    return (currentChunk() -> count) - 2;
}

// goes back into the bytecode and replaces the operand...
// ... at the given location w/ the calculated jump offset
static void patchJump(int offset) {
    // number of bytes we are going to jump by
    int jump = currentChunk() -> count - offset - 2;

    if (jump > UINT16_MAX)
        error("too much code to jump over");
    
    // replacing the placeholder offset with the real offset
    currentChunk() -> code[offset] = (jump >> 8) & 0xff;
    currentChunk() -> code[offset + 1] = jump & 0xff;
}

static void emitLoop(int loopStart) {
    emitByte(OP_LOOP);

    int offset = currentChunk() -> count - loopStart + 2;
    if (offset > UINT16_MAX)
        error("loop body too large");
    
    emitByte((offset >> 8) & 0xff);
    emitByte(offset & 0xff);
}

// starts up the compiler
static void initCompiler(Compiler* compiler) {
    compiler -> localCount = 0;
    compiler -> scopeDepth = 0;
    current = compiler;
}

// ends the compiler
static void endCompiler(void) {
    emitReturn();

// print out chunk's bytecode...
// ... only if there is no error
#ifdef DEBUG_PRINT_CODE
    if (!parser.hadError)
        disassembleChunk(currentChunk(), "code");
#endif

}

// beginning a scope means...
// ... incrementing the current depth
static void beginScope(void) {
    current -> scopeDepth++;
}

// ending a scope means...
// ... decrementing the current depth
static void endScope(void) {
    current -> scopeDepth--;

    // when we pop a scope, we walk backward thru the local array...
    // ... looking for any variables declared at the scope depth we just left
    while(current -> localCount > 0 &&
          current -> locals[current -> localCount - 1].depth > current -> scopeDepth) {
        emitByte(OP_POP);
        current -> localCount--;
    }
}

// function annotations
static void expression(void);
static void statement(void);
static void declaration(void);
static ParseRule* getRule(TokenType type);
static void parsePrecedence(Precedence precedence);
static uint8_t identifierConstant(Token* name);
static int resolveLocal(Compiler* compiler, Token* name);
static void and_(bool canAssign);

static void binary(bool canAssign) {
    TokenType operatorType = parser.previous.type;
    ParseRule* rule = getRule(operatorType);

    // use `+ 1` bc binary operators are left-associative
    parsePrecedence((Precedence) (rule -> precedence + 1));

    switch(operatorType) {
        case TOKEN_BANG_EQUAL:
            emitByte(OP_NOT_EQUAL);
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

static void literal(bool canAssign) {
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
static void grouping(bool canAssign) {
    expression();
    consume(TOKEN_RIGHT_PAREN, "expected ')' after expression");
}

static void number(bool canAssign) {
    // take lexeme stored in previous and use...
    // ... C STL to convert it to a double value
    double value = strtod(parser.previous.start, NULL);
    
    // generate opcode to load value
    //     - wrap C double in a Value struct before...
    //       ... storing it in the constant table
    emitConstant(NUMBER_VAL(value));
}

static void or_(bool canAssign) {
    int elseJump = emitJump(OP_JUMP_IF_FALSE);
    int endJump = emitJump(OP_JUMP);

    patchJump(elseJump);
    emitByte(OP_POP);

    parsePrecedence(PREC_OR);
    patchJump(endJump);
}

// takes string's chars directly from the lexeme and...
// ... creates a string obj which is wrapped in a Value
static void string(bool canAssign) {
    // +1 and -2 trims leading and trailing quotation marks
    emitConstant(OBJ_VAL(copyString(parser.previous.start + 1, parser.previous.length - 2)));
}

static void namedVariable(Token name, bool canAssign) {
    uint8_t getOp, setOp;

    // try to find a local variable with the given name
    int arg = resolveLocal(current, &name);
    
    // local opcodes if a local variable found
    if (arg != -1) {
        getOp = OP_GET_LOCAL;
        setOp = OP_SET_LOCAL;
    }

    // otherwise global opcodes
    else {
        arg = identifierConstant(&name);
        getOp = OP_GET_GLOBAL;
        setOp = OP_SET_GLOBAL;
    }
    
    // equal sign after the identifier means we compile the...
    // ...  assigned value and then emit an assignment instruction
    if (canAssign && match(TOKEN_EQUAL)) {
        expression();
        emitBytes(setOp, (uint8_t)arg);
    }
    // emit access instruction
    else
        emitBytes(getOp, (uint8_t)arg);
}

static void variable(bool canAssign) {
    namedVariable(parser.previous, canAssign);
}

static void unary(bool canAssign) {
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
    [TOKEN_AND]             = {NULL,        and_,   PREC_NONE},
    [TOKEN_CLASS]           = {NULL,        NULL,   PREC_NONE},
    [TOKEN_ELSE]            = {NULL,        NULL,   PREC_NONE},
    [TOKEN_FALSE]           = {literal,     NULL,   PREC_NONE},
    [TOKEN_FOR]             = {NULL,        NULL,   PREC_NONE},
    [TOKEN_FUN]             = {NULL,        NULL,   PREC_NONE},
    [TOKEN_IF]              = {NULL,        NULL,   PREC_NONE},
    [TOKEN_NIL]             = {literal,     NULL,   PREC_NONE},
    [TOKEN_OR]              = {NULL,        or_,   PREC_NONE},
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

    // passing along info that precedence is...
    // ... low enough to allow assignment
    bool canAssign = precedence <= PREC_ASSIGNMENT;
    prefixRule(canAssign);

    // looking for an infix parser for the next token
    while(precedence <= (getRule(parser.current.type) -> precedence)) {
        advance();
        ParseFn infixRule = getRule(parser.previous.type) -> infix;
        infixRule(canAssign);
    }

    if (canAssign && match(TOKEN_EQUAL))
        error("invalid assignment target");
}

// takes the given token and adds its lexeme...
// ... to the chunk's constant table as a string...
// ... and then returns the index of the constant...
// ... in the constant table
static uint8_t identifierConstant(Token* name) {
    return makeConstant(OBJ_VAL(copyString(name -> start,
                                           name -> length)));
}

// returns true if both identifiers are the same...
// ... otherwise false
static bool identifiersEqual(Token* a, Token* b) {
    // check the length of both lexemes
    if (a -> length != b -> length)
        return false;
    
    // check the characters
    return memcmp(a -> start, b -> start, a -> length) == 0;
}

// resolves a local variable
static int resolveLocal(Compiler* compiler, Token* name) {
    // walk the list of locals currently in scope
    for(int i = compiler -> localCount - 1; i >= 0; i--) {
        Local* local = &compiler -> locals[i];

        // found same identifier
        if (identifiersEqual(name, &local -> name)) {
            // variable is uninitialized
            if (local -> depth == -1)
                error("can't read local variable in its own initializer");
            return i;
        }
    }

    return -1;
}

static void addLocal(Token name) {
    if (current -> localCount == UINT8_COUNT) {
        error("too many local variables in function");
        return;
    }

    // initializes next avail Local in...
    // ... the compiler's array of variables
    Local* local = &current -> locals[current -> localCount++];
    
    // store the variable's name and the depth...
    // ... of the scope that owns the variable
    local -> name = name;

    // indicates that the local ...
    // ... variable is uninitialized
    local -> depth = -1;
}

// compiler records the existence of the variable
static void declareVariable(void) {
    // bail out at the top-lvl global scope
    if (current -> scopeDepth == 0)
        return;
    
    // add local variable to compiler's list...
    // ... of variables in the current scope
    Token* name = &(parser.previous);

    // look for an existing variable w/ the same name
    for(int i = current -> localCount - 1; i >= 0; i--) {
        // current scope is always the end of the array
        Local* local = &current -> locals[i];
        if (local -> depth != -1 && local -> depth < current -> scopeDepth)
            break;

        // found variable w/ same name in the current scope 
        if (identifiersEqual(name, &local -> name))
            error("already a variable with this name in this scope");
    }

    addLocal(*name);
}

// requires next token to be an identifier
static uint8_t parseVariable(const char* errorMsg) {
    // requires next token to be an identifier
    consume(TOKEN_IDENTIFIER, errorMsg);

    // "declare" the variable
    declareVariable();

    // exit the function if in a local scope
    if (current -> scopeDepth > 0)
        return 0;

    // index from constant table
    return identifierConstant(&parser.previous);
}

// marks local variable as initialized
static void markInitialized(void) {
    current -> locals[current -> localCount - 1].depth = 
        current -> scopeDepth;
}

// outputs the bytecode instruction that defines...
// ... the new variable and stores its initial value
static void defineVariable(uint8_t global) {
    // exit if in a local scope...
    if (current -> scopeDepth > 0) {
        // marks the variable as initialized
        markInitialized();
        return;
    }

    // ... otherwise output bytecode instruction...
    // ... that defines the new variable...
    // ... and stores its initial value
    emitBytes(OP_DEFINE_GLOBAL, global);
}

static void and_(bool canAssign) {
    // lhs has already been compiled so we...
    // ... can skip the rhs operand if the...
    // ... the value is falsey
    int endJump = emitJump(OP_JUMP_IF_FALSE);

    // otherwise we discard the lhs and...
    // ... evaluate the result of the...
    // ... entire expression
    emitByte(OP_POP);
    parsePrecedence(PREC_AND);

    patchJump(endJump);
}

static ParseRule* getRule(TokenType type) {
    return &rules[type];
}

static void expression(void) {
    // simply parse the lowest precedence lvl
    parsePrecedence(PREC_ASSIGNMENT);
}

static void block(void) {
    // keeps parsing declarations until the closing brace...
    // ... has been hit
    while(!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF))
        declaration();
    
    consume(TOKEN_RIGHT_BRACE, "expected '}' after block");
}

static void varDeclaration(void) {
    // variable name is compiled here
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
    // evaluate the expression...
    expression();
    consume(TOKEN_SEMICOLON, "expected ';' after expression");

    // ... and then discard the result
    emitByte(OP_POP);
}

static void printStatement(void) {
    // evaluate expression
    expression();
    
    consume(TOKEN_SEMICOLON, "expected ';' after value");

    // print the result
    emitByte(OP_PRINT);
}

static void forStatement(void) {
    beginScope();

    consume(TOKEN_LEFT_PAREN, "expect '(' after 'for'");
    if (match(TOKEN_SEMICOLON)) {
        // no initializer
    }
    else if (match(TOKEN_VAR))
        varDeclaration();
    else
        expressionStatement();
    consume(TOKEN_SEMICOLON, "expected ';'");

    int loopStart = currentChunk() -> count;
    int exitJump = -1;
    if (!match(TOKEN_SEMICOLON)) {
        expression();
        consume(TOKEN_SEMICOLON, "expected ';' after loop condition");

        // jump out of the loop if the condition is false
        exitJump = emitJump(OP_JUMP_IF_FALSE);
        emitByte(OP_POP);   // condition
    }
    
    if (!match(TOKEN_RIGHT_PAREN)) {
        int bodyJump = emitJump(OP_JUMP);
        int incrementStart = currentChunk() -> count;
        expression();
        emitByte(OP_POP);
        consume(TOKEN_RIGHT_PAREN, "expected ')' after for clauses");

        emitLoop(loopStart);
        loopStart = incrementStart;
        patchJump(bodyJump);
    }

    statement();
    emitLoop(loopStart);

    if (exitJump != -1) {
        patchJump(exitJump);
        emitByte(OP_POP);   // condition
    }

    endScope();
}

static void ifStatement(void) {
    // compile condition expression, bracketed by "()"
    consume(TOKEN_LEFT_PAREN, "expected '(' after 'if'");
    expression();   // @runtime leaves condition value on top of stack
    consume(TOKEN_RIGHT_PAREN, "expected ')' after condition");

    // emit jmp instruction 1st w/ a placeholder offset operand...
    // ... and keep track of where the half-finished instruction is
    int thenJump = emitJump(OP_JUMP_IF_FALSE);

    // the if statement's condition was true...
    // ... so we pop `true` off the stack
    emitByte(OP_POP);
    
    // compile the if statement's body
    statement();

    // emit jmp instruction if body has...
    // ... been executed that goes over the else
    int elseJump = emitJump(OP_JUMP);

    // replace placeholder offset with the real one...
    // ... since we know how far to jump now
    patchJump(thenJump);

    // the if statement's condition was false...
    // ... so we pop `false` off the stack
    emitByte(OP_POP);

    // after compiling the body, we look for the else keyword...
    // ... if found, we compile the else statement's body
    if (match(TOKEN_ELSE))
        statement();
    
    patchJump(elseJump);
}

static void whileStatement(void) {
    int loopStart = currentChunk() -> count;

    consume(TOKEN_LEFT_PAREN, "expected '(' after 'while'");
    expression();
    consume(TOKEN_RIGHT_PAREN, "expected ')' after condition");

    int exitJump = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP);
    statement();

    emitLoop(loopStart);

    patchJump(exitJump);
    emitByte(OP_POP);
}

// compiles a single declaration, called...
// ... functions end in...
// ... consume(TOKEN_SEMICOLON,"...");
static void statement(void) {
    if (match(TOKEN_PRINT))
        printStatement();
    else if (match(TOKEN_FOR))
        forStatement();
    else if (match(TOKEN_IF))
        ifStatement();
    else if (match(TOKEN_WHILE))
        whileStatement();
    else if (match(TOKEN_LEFT_BRACE)) {
        beginScope();
        block();
        endScope();
    }
    else
        expressionStatement();
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

// returns whether or not compilation succeeded
// compiler writes opcode into `chunk`
bool compile(const char* src, Chunk* chunk) {
    initScanner(src);

    Compiler compiler;
    initCompiler(&compiler);

    compilingChunk = chunk;

    parser.hadError = false;
    parser.panicMode = false;

    // "primes the pump" on the scanner
    advance();

    // a program is a sequence of declarations
    while(!match(TOKEN_EOF))
        declaration();

    endCompiler();

    // returns false if an error occured
    return !parser.hadError;
}
