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

static bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

// src strings ends with a null byte character
static bool isAtEnd() {
    return *scanner.current == '\0';
}

// consumes next char and returns it
static char advance() {
    scanner.current++;
    return scanner.current[-1];
}

// returns the next char w/ no consumption
static char peek() {
    return *scanner.current;
}

// similar to peek but looks at...
// ... the char past the current one
static char peekNext() {
    if (isAtEnd())
        return '\0';
    return scanner.current[1];
}

static bool match(char expected) {
    // if next character is EOF or not expected
    if (isAtEnd())
        return false;
    if (*scanner.current != expected)
        return false;
    
    // advance the scanner
    scanner.current++;

    return true;
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

static void skipWhitespace() {
    for(;;) {
        char c = peek();
        switch(c) {
            case ' ':
            case '\r':
            case '\t':
                advance();
                break;
            case '\n':
                scanner.line++;
                advance();
                break;
            case '/':
                // a comment goes until the end of the line
                if (peekNext() == '/') {
                    while(peek() != '\n' && !isAtEnd())
                        advance();
                }
                else
                    return;
                break;
            default:
                return;
        }
    }
}

static Token number() {
    while(isDigit(peek()))
        advance();
    
    // look for a fractional part
    if (peek() == '.' && isDigit(peekNext())) {
        // consume the '.'
        advance();

        while(isDigit(peek()))
            advance();
    }

    return makeToken(TOKEN_NUMBER);
}

static Token string() {
    // consume chars until we reach the closing quote
    while(peek() != '"' && isAtEnd()) {
        // tracking newlines for multi-line strings
        if (peek() == '\n')
            scanner.line++;
        advance();
    }

    if (isAtEnd())
        return errorToken("unterminated string");
    
    // the closing quote
    advance();
    
    return makeToken(TOKEN_STRING);
}

Token scanToken() {
    // for handling whitespace
    skipWhitespace();

    // pts to current char to remember where the...
    // ... lexeme that is about to be scanned starts
    scanner.start = scanner.current;

    // check if at the end of the src file
    if (isAtEnd())
        return makeToken(TOKEN_EOF);

    char c = advance();

    if (isDigit(c))
        return number();

    switch(c) {
        // single character tokens
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
        
        // one/two character tokens
        case '!':
            return makeToken(
                match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG
            );
        case '=':
            return makeToken(
                match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL
            );
        case '<':
            return makeToken(
                match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS
            );
        case '>':
            return makeToken(
                match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER
            );
        
        // literal tokens
        case '"':
            return string();
    }
    
    return errorToken("unexpected character");
}