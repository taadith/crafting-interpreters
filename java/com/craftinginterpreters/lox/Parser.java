package com.craftinginterpreters.lox;

import java.util.List;

class Parser {
    private final List<Token> tokens;
    private int current = 0;

    Parser(List<Token> tokens) {
        this.tokens = tokens;
    }

    // expression -> equality ;
    private Expr expression() {
        return equality();
    }

    // all binary operators....

    // equality -> comparison (( "!=" | "==" ) comparison )* ;
    private Expr equality() {
        Expr expr = comparison();

        // `(( "!=" | "==" ) comparison )*`
        while (match(TokenType.BANG_EQUAL, TokenType.EQUAL_EQUAL)) {
            Token operator = previous();
            Expr right = comparison();
            expr = new Expr.Binary(expr, operator, right);
        }

        return expr;
    }

    // comparison -> term (( ">" | ">=" | "<" | "<=" ) term )* ;
    private Expr comparison() {
        Expr expr = term();

        // `(( ">" | ">=" | "<" | "<=" ) term )*`
        while (match(TokenType.GREATER, TokenType.GREATER_EQUAL, TokenType.LESS, TokenType.LESS_EQUAL)) {
            Token operator = previous();
            Expr right = term();
            expr = new Expr.Binary(expr, operator, right);
        }

        return expr;
    }

    // term -> factor (( "-" | "+" ) factor )* ;
    private Expr term() {
        Expr expr = factor();

        // `(( "-" | "+" ) factor )*`
        while (match(TokenType.MINUS, TokenType.PLUS)) {
            Token operator = previous();
            Expr right = factor();
            expr = new Expr.Binary(expr, operator, right);
        }

        return expr;
    }

    // factor -> unary (( "/" | "*" ") unary )* ;
    private Expr factor() {
        Expr expr = unary();

        // `(( "/" | "*" ") unary )`
        while (match(TokenType.SLASH, TokenType.STAR)) {
            Token operator = previous();
            Expr right = unary();
            expr = new Expr.Binary(expr, operator, right);
        }

        return expr;
    }

    // unary -> ( "!" | "-" ) unary
    //          | primary ;
    private Expr unary() {
        if (match(TokenType.BANG, TokenType.MINUS)) {
            Token operator = previous();
            Expr right = unary();
            return new Expr.Unary(operator, right);
        }

        return primary();
    }

    // primary -> NUMBER | STRING | "true" | "false" | "nil"
    //            | "(" expression ")" ;
    private Expr primary() {
        if (match(TokenType.FALSE))
            return new Expr.Literal(false);
        if (match(TokenType.TRUE))
            return new Expr.Literal(true);
        if (match(TokenType.NIL))
            return new Expr.Literal(null);
            
        if (match(TokenType.NUMBER, TokenType.STRING))
            return new Expr.Literal(previous().literal);
        
        if (match(TokenType.LEFT_PAREN)) {
            Expr expr = expression();
            consume(TokenType.RIGHT_PAREN, "expected ')' after expression");
            return new Expr.Grouping(expr);
        }
    }

    // checks if the current token has any of the given types
    private boolean match(TokenType... types) {

        // checks if current token has any of the given types
        for (TokenType type : types) {

            // if so, the token is consumed and true is returned
            if (check(type)) {
                advance();
                return true;
            }
        }

        // otherwise, returns false and the current token is left as it is
        return false;
    }

    // checks if the current token is "type"
    private boolean check(TokenType type) {
        if (isAtEnd())
            return false;
        return peek().type == type;
    }

    private Token advance() {
        if (!isAtEnd())
            current++;
        return previous();
    }

    // returns the most recently consumed token
    private Token previous() {
        return tokens.get(current - 1);
    }

    // check if we've run out of tokens to parse
    private boolean isAtEnd() {
        return peek().type == TokenType.EOF;
    }

    // returns current token we have YET to consume
    private Token peek() {
        return tokens.get(current);
    }
}