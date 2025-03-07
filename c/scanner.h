#ifndef clox_scanner_h
#define clox_scanner_h

typedef enum {
    // single-character tokens
    TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,
    TOKEN_COMMA, TOKEN_DOT, TOKEN_MINUS, TOKEN_PLUS,
    TOKEN_SEMICOLON, TOKEN_SLASH, TOKEN_STAR,

    // one or two character tokens
    TOKEN_BANG, TOKEN_BANG_EQUAL,
    TOKEN_EQUAL, TOKEN_EQUAL_EQUAL,
    TOKEN_GREATER, TOKEN_GREATER_EQUAL,
    TOKEN_LESS, TOKEN_LESS_EQUAL,

    // literals
    TOKEN_IDENTIFIER, TOKEN_STRING, TOKEN_NUMBER,

    // keywords
    TOKEN_AND, TOKEN_CLASS, TOKEN_ELSE, TOKEN_FALSE,
    TOKEN_FOR, TOKEN_FUN, TOKEN_IF, TOKEN_NIL, TOKEN_OR,
    TOKEN_PRINT, TOKEN_RETURN, TOKEN_SUPER, TOKEN_THIS,
    TOKEN_TRUE, TOKEN_VAR, TOKEN_WHILE,

    // misc.
    TOKEN_ERROR, TOKEN_EOF
} TokenType;

typedef struct {
    // enum IDs type of token it is
    TokenType type;

    const char* start;
    int length;
    int line;
} Token;

typedef struct {
    // marks begining of current lexeme being scanned
    const char* start;

    // pts to the current char being looked at
    const char* current;

    // for error reporting
    int line;
} Scanner;

void initScanner(Scanner* scanner, const char* src);

Token scanToken(Scanner* scanner);

#endif
