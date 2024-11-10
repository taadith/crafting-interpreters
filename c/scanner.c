#include <stdio.h>
#include <string.h>

#include "common.h"
#include "scanner.h"

typedef struct {
    const char* start;
    const char* current;
    int line;
} Scanner;

Scanner scanner;

void initScanner(const char* src) {
    // start at the very first char...
    scanner.start = src;
    scanner.current = src;

    // ... on the very first line
    scanner.line = 1;
}

// src strings ends with a null byte character
static bool isAtEnd() {
    return *scanner.current == '\0';
}

// constructor-like function for Token
static Token makeToken(TokenType type) {
    Token token;
    token.type = type;

    // uses scanner's start and current ptrs...
    // ... to capture the token's lexeme
    token.start = scanner.start;
    token.length = (int)(scanner.current - scanner.start);
    
    token.line = scanner.line;
    return token;
}

static Token errorToken(const char* msg) {
    Token token;
    token.type = TOKEN_ERROR;
    token.start = msg;
    token.length = (int)strlen(msg);
    token.line = scanner.line;
    return token;
}

Token scanToken() {
    // pts to current char to remember where the...
    // ... lexeme that is about to be scanned starts
    scanner.start = scanner.current;

    // check if at the end of the src file
    if (isAtEnd())
        return makeToken(TOKEN_EOF);
    
    return errorToken("unexpected character");
}