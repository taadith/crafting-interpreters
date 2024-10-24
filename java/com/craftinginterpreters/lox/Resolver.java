package com.craftinginterpreters.lox;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Stack;

class Resolver implements Expr.Visitor<Void>, Stmt.Visitor<Void> {
    private final Interpreter interpreter;

    // only used for local block scopes
    private final Stack<Map<String, Boolean>> scopes = new Stack<>();

    Resolver(Interpreter interpreter) {
        this.interpreter = interpreter;
    }

    // lexical scopes nest in both the interpreter and resolver...
    // ... behaving like a stack
    private void beginScope() {
        scopes.push(new HashMap<String, Boolean>());
    }

    private void endScope() {
        scopes.pop();
    }

    @Override
    public Void visitBlockStmt(Stmt.Block stmt) {
        beginScope();
        resolve(stmt.statements);
        endScope();

        return null;
    }

    @Override
    public Void visitExpressionStmt(Stmt.Expression stmt) {
        resolve(stmt.expression);

        return null;
    }

    // static analysis, so we resolve both branches
    @Override
    public Void visitIfStmt(Stmt.If stmt) {
        resolve(stmt.condition);
        resolve(stmt.thenBranch);

        if (stmt.elseBranch != null)
            resolve(stmt.elseBranch);

        return null;
    }

    @Override
    public Void visitFunctionStmt(Stmt.Function stmt) {
        declare(stmt.name);
        define(stmt.name);

        resolveFunction(stmt);

        return null;
    }

    @Override
    public Void visitPrintStmt(Stmt.Print stmt) {
        resolve(stmt.expression);

        return null;
    }

    @Override
    public Void visitReturnStmt(Stmt.Return stmt) {
        if (stmt.value != null)
            resolve(stmt.value);
        
        return null;
    }

    @Override
    public Void visitVarStmt(Stmt.Var stmt) {
        declare(stmt.name);
        if (stmt.initializer != null)
            resolve(stmt.initializer);
        define(stmt.name);
        return null;
    }

    @Override
    public Void visitWhileStmt(Stmt.While stmt) {
        resolve(stmt.condition);
        resolve(stmt.body);

        return null;
    }

    @Override
    public Void visitAssignExpr(Expr.Assign expr) {
        // resolve expression for the assigned value...
        // ...in case it contains refs to other vars
        resolve(expr.value);

        // resolve the variable that's being assigned to
        resolveLocal(expr, expr.name);

        return null;
    }

    @Override
    public Void visitVariableExpr(Expr.Variable expr) {
        boolean declared_and_not_defined = !scopes.isEmpty() && 
                            scopes.peek().get(expr.name.lexeme) == Boolean.FALSE;
        if (declared_and_not_defined)
            Lox.error(expr.name, "can't read local variable in its own initializer");
        
        // resolve the variable itself using this helper
        resolveLocal(expr, expr.name);
        
        return null;
    }

    @Override
    public Void visitBinaryExpr(Expr.Binary expr) {
        resolve(expr.left);
        resolve(expr.right);

        return null;
    }

    @Override
    public Void visitCallExpr(Expr.Call expr) {
        resolve(expr.callee);

        for (Expr arg : expr.args)
            resolve(arg);

        return null;
    }

    @Override
    public Void visitGroupingExpr(Expr.Grouping expr) {
        resolve(expr.expression);

        return null;
    }

    @Override
    public Void visitLiteralExpr(Expr.Literal expr) {
        return null;
    }

    @Override
    public Void visitLogicalExpr(Expr.Logical expr) {
        resolve(expr.left);
        resolve(expr.right);

        return null;
    }

    @Override
    public Void visitUnaryExpr(Expr.Unary expr) {
        resolve(expr.right);
        
        return null;
    }

    // adds var to the innermost scope ...
    // ... so that it shadows any outer one ...
    // ... and so that we know the var exists
    private void declare(Token name) {
        if (scopes.isEmpty())
            return;
        
        Map<String, Boolean> scope = scopes.peek();

        // marking it as not ready yet
        scope.put(name.lexeme, false);
    }

    private void define(Token name) {
        if (scopes.isEmpty())
            return;
        
        // marked as fully initialized and avail for use
        scopes.peek().put(name.lexeme, true);
    }
    
    private void resolve(List<Stmt> stmts) {
        for (Stmt stmt : stmts)
            resolve(stmt);
    }

    private void resolve(Stmt stmt) {
        stmt.accept(this);
    }

    private void resolve(Expr expr, int depth) {
        locals.put(expr, depth);
    }
    
    private void resolveFunction(Stmt.Function function) {
        // creates a new scope for the body
        beginScope();
        
        // binds vars for each of the function's params
        for (Token param : function.params) {
            declare(param);
            define(param);
        }

        // resolve function body in that scope
        resolve(function.body);

        endScope();
    }

    private void resolveLocal(Expr expr, Token name) {
        for (int i = scopes.size() - 1; i >= 0; i--) {
            if (scopes.get(i).containsKey(name.lexeme)) {
                interpreter.resolve(expr, scopes.size() - 1 - i);
                return;
            }
        }
    }
}