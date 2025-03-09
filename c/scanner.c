#include <stdio.h>
#include <string.h>

#include "common.h"
#include "scanner.h"

void initScanner(Scanner* scanner, const char* src) {
    scanner -> start = src;
    scanner -> current = src;
    scanner -> line = 0;
}

static bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

// checks for null-byte char
static bool isAtEnd(Scanner* scanner) {
    return *(scanner -> current) == '\0';
}

// reads next char from src code
static char advance(Scanner* scanner) {
    scanner -> current++;

    // same as *(scanner -> current - 1)
    return scanner -> current[-1];
}

// returns current char w/out consumption
static char peek(Scanner* scanner) {
    return *(scanner -> current);
}

// peeks at char past the current one
static char peekNext(Scanner* scanner) {
    if (isAtEnd(scanner))
        return '\0';

    // same as *(scanner -> current + 1)
    return scanner -> current[1];
}

static bool match(Scanner* scanner, char expected) {
    // next char is null byte
    if (isAtEnd(scanner))
        return false;

    // next char is something else
    if (*(scanner -> current) != expected)
        return false;

    // desired char was found so we advance...
    // ... and return true
    scanner -> current++;
    return true;
}

// constructor-like function that creates...
// ... a token
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

// advances the scanner past any leading whitespace
static void skipWhitespace(Scanner* scanner) {
    for(;;) {
        char c = peek(scanner);
        switch (c) {
            case ' ':
            case '\r':

            case '\t':
                advance(scanner);
                break;

            case '\n':
                scanner -> line++;
                advance(scanner);
                break;

            case '/':
                if (peekNext(scanner) == '/') {
                    // comments go until EOL
                    while(peek(scanner) != '\n' && !isAtEnd(scanner))
                        advance(scanner);
                }
                else
                    return;
                break;

            default;
                return;
        }
    }
}

// create a number lexeme
static Token number(Scanner* scanner) {
    while(isDigit(peek(scanner))
          advance(scanner);

    // look for a fractional part
    if (peek(scanner) == '.' && isDigit(peekNext(scanner))) {
        // consume the '.'
        advance(scanner);

        while (isDigit(peek(scanner)))
            advance(scanner);
    }

    return makeToken(scanner, TOKEN_NUMBER);
}

// create a string lexeme
static Token string(Scanner* scanner) {
    // consume chars until closing quote
    while(peek(scanner) != '"' && !isAtEnd()) {
        if (peek(scanner) == '\n')
            scanner -> line++;
        advance(scanner);
    }

    if (isAtEnd(scanner))
        return errorToken(scanner, "unterminated string");

    // closing quote
    advance(scanner);

    return makeToken(scanner, TOKEN_STRING);
}

Token scanToken(Scanner* scanner) {
    // advances the scanner past any leading whitespace
    skipWhitespace(scanner);

    scanner -> start = scanner -> current;
    
    if (isAtEnd(scanner))
        return makeToken(scanner, TOKEN_EOF);

    char c = advance(scanner);

    if (isDigit(c))
        return number(scanner);

    switch(c) {
        // single-character tokens
        case '(':
            return makeToken(scanner, TOKEN_LEFT_PAREN);
        case ')':
            return makeToken(scanner, TOKEN_RIGHT_PAREN);
        case '{':
            return makeToken(scanner, TOKEN_LEFT_BRACE);
        case '}':
            return makeToken(scanner, TOKEN_RIGHT_BRACE);
        case ';':
            return makeToken(scanner, TOKEN_SEMICOLON);
        case ',':
            return makeToken(scanner, TOKEN_COMMA);
        case '.':
            return makeToken(scanner, TOKEN_DOT);
        case '-':
            return makeToken(scanner, TOKEN_MINUS);
        case '+':
            return makeToken(scanner, TOKEN_PLUS);
        case '/':
            return makeToken(scanner, TOKEN_SLASH);
        case '*':
            return makeToken(scanner, TOKEN_STAR);
        
        // one/two-character tokens
        case '!':
            return makeToken(scanner,
                match(scanner, '=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
        case '=':
            return makeToken(scanner,
                match(scanner, '=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
        case '<':
            return makeToken(scanner,
                match(scanner, '=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
        case '>':
            return makeToken(scanner,
                match(scanner, '=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);

        // literal tokens
        case '"':
            return string(scanner);
    }
    
    return errorToken(scanner, "unexpected character");
}
