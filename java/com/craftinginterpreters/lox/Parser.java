package com.craftinginterpreters.lox;

import java.util.ArrayList;
import java.util.List;

class Parser {
    // simple sentinel class used to unwind the parser
    private static class ParseError extends RuntimeException {}

    private final List<Token> tokens;
    private int current = 0;

    Parser(List<Token> tokens) {
        this.tokens = tokens;
    }
    
    // program -> declaration* EOF ;
    List<Stmt> parse() {
        List<Stmt> statements = new ArrayList<>();

        while(!isAtEnd())
            statements.add(declaration());
        
        return statements;
    }

    // declaration -> varDecl
    //                | statement ;
    private Stmt declaration() {
        try {
            // `"var" `
            if (match(TokenType.VAR))
                return varDeclaration();
            
            return statement();
        } catch (ParseError error) {
            synchronize();
            return null;
        }
    }

    // varDecl -> IDENTIFIER ( "=" expression )? ";" ;
    private Stmt varDeclaration() {
        Token name = consume(TokenType.IDENTIFIER, "expected a variable name");

        Expr initializer = null;
        if (match(TokenType.EQUAL))
            initializer = expression();
        
        consume(TokenType.SEMICOLON, "expected ';' after variable declaration");
        return new Stmt.Var(name, initializer);
    }

    // statement -> exprStatement
    //              | printStmt ;
    private Stmt statement() {
        if (match(TokenType.PRINT))
            return printStatement();
        
        return expressionStatement();
    }

    // exprStmt -> expression ( "," expression )* ";" ;
    private Stmt expressionStatement() {
        Expr expr = expression();

        //`( "," expression )*`
        while (match(TokenType.COMMA)) {
            Token operator = previous();
            Expr right = expression();
            expr = new Expr.Binary(expr, operator, right);
        }

        consume(TokenType.SEMICOLON, "expected ';' after expression");
        return new Stmt.Expression(expr);
    }

    // printStmt -> "print" expression ";" ;
    private Stmt printStatement() {
        Expr value = expression();
        consume(TokenType.SEMICOLON, "expected ';' after value");
        return new Stmt.Print(value);
    }

    // expression -> assignment ;
    private Expr expression() {
        return assignment();
    }

    // assignment -> IDENTIFIER "=" assignment
    //               | conditional ;
    private Expr assignment() {
        Expr expr = conditional();

        if (match(TokenType.EQUAL)) {
            Token equals = previous();
            Expr value = assignment();

            if (expr instanceof Expr.Variable) {
                Token name = ((Expr.Variable) expr).name;
                return new Expr.Assign(name, value);
            }

            error(equals, "invalid assignment target");
        }

        return expr;
    }

    // ternary conditional operator

    // conditional -> equality ( "?" conditional ":" conditional )* ;
    private Expr conditional() {
        // in case of `"?" conditional ":" conditional`
        if (match(TokenType.QUESTION)) {
            // "?" expression
            error(previous(), "unexpected '?' w/out left-hand operand");
            conditional();

            // `":" conditional`
            consume(TokenType.COLON, "expected ':' after expression");
            conditional();

            return null;
        }

        Expr expr = equality();

        // `( "?" conditional ":" conditional )*`
        while(match(TokenType.QUESTION)) {
            Token op1 = previous();
            Expr mid = conditional();

            consume(TokenType.COLON, "expected ':' after second conditional expression");

            Token op2 = previous();
            Expr right = conditional();
            
            expr = new Expr.Ternary(expr, op1, mid, op2, right);
        }

        return expr;
    }

    // all binary operators....

    // equality -> comparison (( "!=" | "==" ) comparison )* ;
    private Expr equality() {
        if (match(TokenType.BANG_EQUAL, TokenType.EQUAL_EQUAL)) {
            error(previous(), "unexpected equality operator w/out left-hand operand");
            comparison();

            return null;
        }

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
        if (match(TokenType.GREATER, TokenType.GREATER_EQUAL, TokenType.LESS, TokenType.LESS_EQUAL)) {
            error(previous(), "unexpected comparison operator w/out left-hand operand");
            term();

            return null;
        }

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
        if (match(TokenType.MINUS, TokenType.PLUS)) {
            error(previous(), "unexpected additive operator w/out left-hand operand");
            factor();

            return null;
        }

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
        if (match(TokenType.SLASH, TokenType.STAR)) {
            error(previous(), "unexpected multiplicative operator w/out left-hand operand");
            unary();

            return null;
        }

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

    // primary -> "false" | "true" | "nil"
    //            | NUMBER | STRING
    //            | IDENTIFIER
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
        
        if (match(TokenType.IDENTIFIER))
            return new Expr.Variable(previous());
        
        if (match(TokenType.LEFT_PAREN)) {
            Expr expr = expression();
            consume(TokenType.RIGHT_PAREN, "expected ')' after expression");
            return new Expr.Grouping(expr);
        }

        throw error(peek(), "expected expression");
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

    // checks if next token is of the expected type
    private Token consume(TokenType type, String msg) {

        // next token is expected so token is consumed and everything is "groovy"
        if (check(type))
            return advance();
        
        throw error(peek(), msg);
    }

    // returns the error instead of throwing it...
    // ... so that the calling method inside the parser can decide whether to unwind or not
    private ParseError error(Token token, String msg) {
        Lox.error(token, msg);
        return new ParseError();
    }

    private void synchronize() {
        advance();

        while(!isAtEnd()) {

            // discard tokens...
            if (previous().type == TokenType.SEMICOLON)
                return;
            
            // ... until we've reached a statement boundary
            switch (peek().type) {
                case CLASS:
                case FUN:
                case VAR:
                case FOR:
                case IF:
                case WHILE:
                case PRINT:
                case RETURN:
                    return;
            }

            advance();
        }
    }
}