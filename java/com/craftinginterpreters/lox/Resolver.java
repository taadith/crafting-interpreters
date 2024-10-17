package com.craftinginterpreters.lox;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Stack;

class Resolver implements Expr.Visitor<Void>, Stmt.Visitor<Void> {
    private final Interpreter interpreter;

    // only used for local block scopes
    private final Stack<Map<String, Boolean>> scopes = new Stack<>();


    // lexical scopes nest in both the interpreter and resolver...
    // ...behaving like a stack
    private void beginScope() {
        scopes.push(new HashMap<String, Boolean>());
    }

    private void endScope() {
        scopes.pop();
    }

    Resolver(Interpreter interpreter) {
        this.interpreter = interpreter;
    }

    @Override
    public Void visitBlockStmt(Stmt.Block stmt) {
        beginScope();
        resolve(stmt.statements);
        endScope();

        return null;
    }

    @Override
    public Void visitVarStmt(Stmt.Var stmt) {
        declare(stmt.name);
        if (stmt.initializer != null)
            resolve(stmt.initalizer);
        define(stmt.name);
        return null;
    }

    // adds var to the innermost scope ...
    // ... so that it shadows any outer one ...
    // ... and so that we know the var exists
    private void declare(Token name) {
        if (scopes.isEmpty())
            return;
        
        Map<String, Boolean> scope = scopes.peek();
        scope.put(name.lexeme, false);
    }
    private void resolve(List<Stmt> stmts) {
        for (Stmt stmt : stmts)
            resolve(stmt);
    }

    private void resolve(Stmt stmt) {
        stmt.accept(this);
    }
}