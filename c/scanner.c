#include <stdio.h>
#include <string.h>

#include "common.h"
#include "scanner.h"

// TODO: make this dynamic
// Scanner scanner;

void initScanner(Scanner* scanner, const char* src) {
    scanner -> start = src;
    scanner -> current = src;
    scanner -> line = 0;
}

static bool isAtEnd(Scanner* scanner) {
    return *(scanner -> current) == '\0';
}

static Token makeToken(Scanner* scanner, TokenType type) {
    Token token;
    token.type = type;

    // scanner's `start` and `current` ptrs used...
    // ... to capture the token's lexeme
    token.start = scanner -> start;
    token.length = (int)(scanner -> current - scanner -> start);

    token.line = scanner -> line;

    return token;
}

static Token errorToken(Scanner* scanner, const char* msg) {
    Token token;
    token.type = TOKEN_ERROR;
    
    // diff from `makeToken()` is that it pts to...
    // ... the error msg, instead of the src code...
    token.start = msg;
    token.length = (int)strlen(msg);

    token.line = scanner -> line;

    return token;
}

Token scanToken(Scanner* scanner) {
    scanner -> start = scanner -> current;
    
    if (isAtEnd(scanner))
        return makeToken(scanner, TOKEN_EOF);

    char c = advance();

    switch(c) {
        // single-character tokens
        case '(':
            return makeToken(TOKEN_LEFT_PAREN);
        case ')':
            return makeToken(TOKEN_RIGHT_PAREN);
        case '{':
            return makeToken(TOKEN_LEFT_BRACE);
        case '}':
            return makeToken(TOKEN_RIGHT_BRACE);
        case ';':
            return makeToken(TOKEN_SEMICOLON);
        case ',':
            return makeToken(TOKEN_COMMA);
        case '.':
            return makeToken(TOKEN_DOT);
        case '-':
            return makeToken(TOKEN_MINUS);
        case '+':
            return makeToken(TOKEN_PLUS);
        case '/':
            return makeToken(TOKEN_SLASH);
        case '*':
            return makeToken(TOKEN_STAR);
    }
    
    return errorToken(scanner, "unexpected character");
}
