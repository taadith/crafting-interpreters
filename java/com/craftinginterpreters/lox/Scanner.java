package com.craftinginterpreters.lox;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

class Scanner {
    private final String src;
    private final List<Token> tokens = new ArrayList<>();

    // reserved keywords
    private static final Map<String, TokenType> keywords;
    static {
        keywords = new HashMap<>();
        keywords.put("and",         TokenType.AND);
        keywords.put("class",       TokenType.CLASS);
        keywords.put("else",        TokenType.ELSE);
        keywords.put("false",       TokenType.FALSE);
        keywords.put("for",         TokenType.FOR);
        keywords.put("fun",         TokenType.FUN);
        keywords.put("if",          TokenType.IF);
        keywords.put("nil",         TokenType.NIL);
        keywords.put("or",          TokenType.OR);
        keywords.put("print",       TokenType.PRINT);
        keywords.put("return",      TokenType.RETURN);
        keywords.put("super",       TokenType.SUPER);
        keywords.put("this",        TokenType.THIS);
        keywords.put("true",        TokenType.TRUE);
        keywords.put("var",         TokenType.VAR);
        keywords.put("while",       TokenType.WHILE);
    }
    
    private int start = 0;
    private int current = 0;
    private int line = 1;

    // store the raw src code as a simple string
    Scanner(String src) {
        this.src = src;
    }

    // takes raw src code
    List<Token> scanTokens() {
        // scan a single token
        while (!isAtEnd()) {
            // we're at the beginning of the next lexeme
            start = current;
            scanToken();
        }

        tokens.add(new Token(TokenType.EOF, "", null, line));
        return tokens;
    }

    private void scanToken() {
        char c = advance();

        switch(c) {
            // single-character tokens
            case '(':
                addToken(TokenType.LEFT_PAREN);
                break;
            case ')':
                addToken(TokenType.RIGHT_PAREN);
                break;
            case '{':
                addToken(TokenType.LEFT_BRACE);
                break;
            case '}':
                addToken(TokenType.RIGHT_BRACE);
                break;
            case ',':
                addToken(TokenType.COMMA);
                break;
            case '.':
                addToken(TokenType.DOT);
                break;
            case '-':
                addToken(TokenType.MINUS);
                break;
            case '+':
                addToken(TokenType.PLUS);
                break;
            case ';':
                addToken(TokenType.SEMICOLON);
                break;
            case '*':
                addToken(TokenType.STAR);
                break;
            case '?':
                addToken(TokenType.QUESTION);
                break;
            case ':':
                addToken(TokenType.COLON);
                break;

            // one or two character tokens
            case '!':
                addToken(match('=') ? TokenType.BANG_EQUAL : TokenType.BANG);
                break;
            case '=':
                addToken(match('=') ? TokenType.EQUAL_EQUAL : TokenType.EQUAL);
                break;
            case '<':
                addToken(match('=') ? TokenType.LESS_EQUAL : TokenType.LESS);
                break;
            case '>':
                addToken(match('=') ? TokenType.GREATER_EQUAL : TokenType.GREATER);
                break;
            case '/':
                // comment goes to EOL
                if (match('/')) {
                    while (peek() != '\n' && !isAtEnd())
                        advance();
                }
                else if (match('*'))
                    multiline_comment();
                else
                    addToken(TokenType.SLASH);
                    break;
            
            // ignore whitespace
            case ' ':
            case '\r':  // carriage return
            case '\t':
                break;
            
            case '\n':
                line++;
                break;
            
            case '"':
                string();
                break;
            
            default:
                if (isDigit(c))
                    number();
                else if(isAlpha(c))
                    identifier();
                else
                    Lox.error(line, "unexpected character");
                break;
        }
    }

    // multiline_comment() -> "/*" * "*/"
    private void multiline_comment() {
        while((peek() != '*' && peekNext() != '/') && !isAtEnd()) {
            if (peek() == '\n')
                line++;
            advance();
        }

        // if we hit the end, we never reached...
        // ... the end of the multiline comment
        if (isAtEnd()) {
            Lox.error(line, "unterminated multiline comment");
            return;
        }

        // the closing characters: '*', '/'
        advance();
        advance();
    }
    
    // string() -> '"' * '"'
    private void string() {
        // consume chars until second '"'...
        // ...or we've hit the end
        while(peek() != '"' && !isAtEnd()) {
            if (peek() == '\n')
                line++;
            advance();
        }

        // if we hit the end, we never reached...
        // ... the end of the string
        if (isAtEnd()) {
            Lox.error(line, "unterminated string");
            return;
        }
        
        // the closing '"'
        advance();

        // trim the surrouding quotes
        String value = src.substring(start + 1, current - 1);
        addToken(TokenType.STRING, value);
    }

    // number() -> (0..9)* ('.'(0..9)*)?
    private void number() {
        // so long as the next char is a digit, "advance"
        while(isDigit(peek()))
            advance();
        
        // looking for decimal pt and mantissa
        if (peek() == '.' && isDigit(peekNext())) {
            // consume '.'
            advance();

            while(isDigit(peek()))
                advance();
        }

        addToken(TokenType.NUMBER,
            Double.parseDouble(src.substring(start, current)));
    }

    private void identifier() {
        while(isAlphaNumeric(peek()))
            advance();
        
        String txt = src.substring(start, current);
        
        // check for a match in hashmap
        TokenType type = keywords.get(txt);
        if (type == null)
            type = TokenType.IDENTIFIER;
        addToken(type);
    }

    private boolean isAtEnd() {
        return current >= src.length();
    }

    // consumes the next char in the src file and returns it
    private char advance() {
        return src.charAt(current++);
    }

    private void addToken(TokenType type) {
        addToken(type, null);
    }

    // grabs txt of current lexeme and creates a new "Token" for it
    private void addToken(TokenType type, Object literal) {
        String txt = src.substring(start, current);
        tokens.add(new Token(type, txt, literal, line));
    }

    // "conditional advance" - only consume char if it is "expected"
    // one char of lookahead
    private boolean match(char expected) {
        if (isAtEnd())
            return false;
        if (src.charAt(current) != expected)
            return false;

        current++;
        return true;
    }

    // similar to advance() w/out consuming the char
    // basically, one char of lookahead
    private char peek() {
        if (isAtEnd())
            return '\0';
        return src.charAt(current);
    }

    private char peekNext() {
        if (current + 1 >= src.length())
            return '\0';
        return src.charAt(current + 1);
    }

    private boolean isDigit(char c) {
        return c >= '0' && c <= '9';
    }

    private boolean isAlpha(char c) {
        return (c >= 'a' && c <= 'z') ||
               (c >= 'A' && c <= 'Z') ||
               c == '_';
    }

    private boolean isAlphaNumeric(char c) {
        return isAlpha(c) || isDigit(c);
    }
}
