package java.com.craftinginterpreters.lox;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import java.com.craftinginterpreters.lox.TokenType;

class Scanner {
    private final String src;
    private final List<Token> tokens = new ArrayList<>();
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
            case '{':
                addToken(TokenType.LEFT_BRACE);
            case '}':
                addToken(TokenType.RIGHT_BRACE);
            case ',':
                addToken(TokenType.COMMA);
            case '.':
                addToken(TokenType.DOT);
            case '-':
                addToken(TokenType.MINUS);
            case '+':
                addToken(TokenType.PLUS);
            case ';':
                addToken(TokenType.SEMICOLON);
            case '*':
                addToken(TokenType.STAR);

            // one or two character tokens
            case '!':
                addToken(match('=') ? TokenType.BANG_EQUAL : TokenType.BANG);
                break;
            case '=':
                addToken(match('=') ? TokenType.EQUAL_EQUAL : TokenType.EQUAL);
                break;
            case '<':
                addToken(match('<') ? TokenType.LESS_EQUAL : TokenType.LESS);
                break;
            case '>':
                addToken(match('>') ? TokenType.GREATER_EQUAL : TokenType.GREATER);
                break;
            
            default:
                Lox.error(line, "unexpected character");
                break;
        }
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

    private boolean match(char expected) {
        if (isAtEnd())
            return false;
        if (src.charAt(current) != expected)
            return false;

        current++;
        return true;
    }
}
