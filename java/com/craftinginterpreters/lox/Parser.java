package com.craftinginterpreters.lox;

import java.util.ArrayList;
import java.util.Arrays;
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

    // declaration -> classDecl
    //                | funDecl
    //                | varDecl
    //                | statement ;
    private Stmt declaration() {
        try {
            // ` "class" `
            if (match(TokenType.CLASS))
                return classDeclaration();
                
            // funDecl -> "fun" function ;
            if (match(TokenType.FUN))
                return function("function");
            
            // ` "var" `
            if (match(TokenType.VAR))
                return varDeclaration();
            
            return statement();
        } catch (ParseError error) {
            synchronize();
            return null;
        }
    }

    // classDecl -> "class" IDENTIFIER "{" function* "}";
    private Stmt classDeclaration() {
        // `IDENTIFIER "{"`
        Token name = consume(TokenType.IDENTIFIER, "expected class name");
        consume(TokenType.LEFT_BRACE, "expected '{' before class body");

        // ` function* `
        List<Stmt.Function> methods = new ArrayList<>();
        while (!check(TokenType.RIGHT_BRACE) && !isAtEnd())
            methods.add(function("method"));
        
        // ` "}" `
        consume(TokenType.RIGHT_BRACE, "expected");

        return new Stmt.Class(name, methods);
    }

    // function -> IDENTIFIER "(" parameters? ")" block ;
    private Stmt.Function function(String kind) {
        Token name = consume(TokenType.IDENTIFIER, "expected " + kind + "name");
        consume(TokenType.LEFT_PAREN, "expected '(' after" + kind + " name");

        // parameters -> IDENTIFIER ( "," IDENTIFIER )* ;
        List<Token> params = new ArrayList<>();
        if (!check(TokenType.RIGHT_PAREN)) {
            do {
                if (params.size() >= 255)
                    error(peek(), "can't have more than 255 parameters");
                
                params.add(
                    consume(TokenType.IDENTIFIER, "expected parameter name")
                );
            } while (match(TokenType.COMMA));
        }

        consume(TokenType.RIGHT_PAREN, "expected ')' after parameters");

        consume(TokenType.LEFT_BRACE, "expected '{' before " + kind + " body");
        List<Stmt> body = block();
        return new Stmt.Function(name, params, body);
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
    //              | forStmt
    //              | ifStmt
    //              | printStmt
    //              | returnStmt
    //              | whileStmt
    //              | block ;
    private Stmt statement() {
        if (match(TokenType.FOR))
            return forStatement();
        
        if (match(TokenType.IF))
            return ifStatement();

        if (match(TokenType.PRINT))
            return printStatement();
        
        if (match(TokenType.RETURN))
            return returnStatement();
        
        if (match(TokenType.WHILE))
            return whileStatement();
        
        if (match(TokenType.LEFT_BRACE))
            return new Stmt.Block(block());
        
        return expressionStatement();
    }

    // forStmt -> "for" "(" ( varDecl | exprStmt ) ";" )
    //            expression? ";"
    //            expression? ")" statement ;
    private Stmt forStatement() {
        // `"("`
        consume(TokenType.LEFT_PAREN, "expected '(' after 'for'");

        // the initializer
        Stmt initializer;
        if (match(TokenType.SEMICOLON)) // would indicate that initializer is omitted
            initializer = null;
        else if (match(TokenType.VAR))
            initializer = varDeclaration();
        else
            initializer = expressionStatement();

        // the condition
        Expr condition = null;
        if(!check(TokenType.SEMICOLON)) // looking for semicolon again for possible omission
            condition = expression();
        consume(TokenType.SEMICOLON, "expected ';' after loop condition");

        // the increment
        Expr increment = null;
        if (!check(TokenType.RIGHT_PAREN))
            increment = expression();
        consume(TokenType.RIGHT_PAREN, "expected ')' after for clauses");
        Stmt body = statement();
        
        if (increment != null) {
            body = new Stmt.Block(
                Arrays.asList(
                    body,
                    new Stmt.Expression(increment)
                )
            );
        }

        if (condition == null)
            condition = new Expr.Literal(true);
        body = new Stmt.While(condition, body);

        if (initializer != null)
            body = new Stmt.Block(Arrays.asList(initializer, body));

        return body;
    }

    // ifStmt -> "if" "(" expression ")" statement
    //           ( "else" statement )? ;
    private Stmt ifStatement() {
        consume(TokenType.LEFT_PAREN, "expected '('' after 'if'");
        Expr condition = expression();
        consume(TokenType.RIGHT_PAREN, "expected ')' after if condition");

        Stmt thenBranch = statement();
        Stmt elseBranch = null;
        if (match(TokenType.ELSE))
            elseBranch = statement();
        
        return new Stmt.If(condition, thenBranch, elseBranch);
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

    // returnStmt -> "return" expression? ";" ;
    private Stmt returnStatement() {
        Token keyword = previous();
        Expr value = null;
        if (!check(TokenType.SEMICOLON))
            value = expression();
        
        consume(TokenType.SEMICOLON, "expected ';' after return value");
        return new Stmt.Return(keyword, value);
    }

    // whileStmt -> "while" "(" expression ")" statement ;
    private Stmt whileStatement() {
        consume(TokenType.LEFT_PAREN, "expected '(' after 'while'");
        Expr condition = expression();

        consume(TokenType.RIGHT_PAREN, "expected ')' after condition");
        Stmt body = statement();

        return new Stmt.While(condition, body);
    }
    
    // block -> "{" declaration* "}" ;
    private List<Stmt> block() {
        List<Stmt> statements = new ArrayList<>();

        // parse stmts until end the end of the block...
        // ... or at EOF
        while (!check(TokenType.RIGHT_BRACE) && !isAtEnd())
            statements.add(declaration());
        
        consume(TokenType.RIGHT_BRACE, "expected '}' after the block");
        return statements;
    }

    // expression -> assignment ;
    private Expr expression() {
        return assignment();
    }

    // assignment -> ( call "." )? IDENTIFIER "=" assignment
    //               | logic_or ;
    private Expr assignment() {
        Expr expr = or();

        if (match(TokenType.EQUAL)) {
            Token equals = previous();
            Expr value = assignment();

            if (expr instanceof Expr.Variable) {
                Token name = ((Expr.Variable) expr).name;
                return new Expr.Assign(name, value);
            }
            else if (expr instanceof Expr.Get) {
                Expr.Get get = (Expr.Get) expr;
                return new Expr.Set(get.object, get.name, value);
            }

            error(equals, "invalid assignment target");
        }

        return expr;
    }

    // logical operators...

    // logic_or -> logic_and ( "or" logic_and )* ;
    private Expr or() {
        Expr expr = and();

        // `( "or" logic_and )*`
        while(match(TokenType.OR)) {
            Token operator = previous();
            Expr right = and();
            expr = new Expr.Logical(expr, operator, right);
        }

        return expr;
    }

    // logic_and -> conditional ( "and" conditional )* ;
    private Expr and() {
        Expr expr = conditional();

        // `( "and" conditional )*`
        while(match(TokenType.AND)) {
            Token operator = previous();
            Expr right = equality();
            expr = new Expr.Logical(expr, operator, right);
        }

        return expr;
    }

    // ternary conditional operator...

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
    //          | call ;
    private Expr unary() {
        if (match(TokenType.BANG, TokenType.MINUS)) {
            Token operator = previous();
            Expr right = unary();
            return new Expr.Unary(operator, right);
        }

        return call();
    }

    // call -> primary ( "(" arguments? ")" | "." IDENTIFIER )* ;
    private Expr call() {
        Expr expr = primary();

        while(true) {
            if (match(TokenType.LEFT_PAREN))
                expr = finishCall(expr);
            else if (match(TokenType.DOT)) {
                Token name = consume(TokenType.IDENTIFIER,
                    "expected property name after '.'");
                expr = new Expr.Get(expr, name);
            }
            else
                break;
        }

        return expr;
    }

    // arguments -> expression ( "," expression)* ;
    private Expr finishCall(Expr callee) {
        List<Expr> args = new ArrayList<>();
        if (!check(TokenType.RIGHT_PAREN)) {
            do {
                if (args.size() >= 255)
                    error(peek(), "can't have more than 255 arguments");
                
                args.add(expression());
            } while (match(TokenType.COMMA));
        }

        Token paren = consume(TokenType.RIGHT_PAREN, "expected ')' after arguments");

        return new Expr.Call(callee, paren, args);
    }

    // primary -> "false" | "true" | "nil"
    //            | NUMBER | STRING
    //            | "this"
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
        
        if (match(TokenType.THIS))
            return new Expr.This(previous());
        
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