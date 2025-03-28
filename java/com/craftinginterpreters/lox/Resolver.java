package com.craftinginterpreters.lox;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Stack;

class Resolver implements Expr.Visitor<Void>, Stmt.Visitor<Void> {
    private final Interpreter interpreter;

    // keeps track of scopes currently "in scope"
    // each element in the stack is a Map representing...
    // ...a single block scope
    private final Stack<Map<String, Boolean>> scopes = new Stack<>();

    private enum ClassType {
        NONE,
        CLASS,
        SUBCLASS
    }

    private enum FunctionType {
        NONE,
        FUNCTION,
        INITIALIZER,
        METHOD
    }

    private ClassType currentClass = ClassType.NONE;

    // extend resolver to detect this statically
    private FunctionType currentFunction = FunctionType.NONE;

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
    public Void visitClassStmt(Stmt.Class stmt) {
        ClassType enclosingClass = currentClass;
        currentClass = ClassType.CLASS;

        declare(stmt.name);
        define(stmt.name);

        if (stmt.superclass != null &&
                stmt.name.lexeme.equals(stmt.superclass.name))
            Lox.error(stmt.superclass.name, "a class can't inherit from itself");

        if (stmt.superclass != null) {
            // we know it has a superclass...
            // ... so this is a subclass
            currentClass = ClassType.SUBCLASS;

            resolve(stmt.superclass);
        }
        
        // if class declaration has a superclass...
        // ... then a new scope is created surrounding...
        // ... all of its methods
        if (stmt.superclass != null) {
            beginScope();
            scopes.peek().put("super", true);
        }

        beginScope();
        scopes.peek().put("this", true);

        // iterate thru methods in class body
        for (Stmt.Function method : stmt.methods) {
            FunctionType declaration = FunctionType.METHOD;

            // use visited method to determine if...
            // ... we're resolving an initializer or not
            if (method.name.lexeme.equals("init"))
                declaration = FunctionType.INITIALIZER;
            
            resolveFunction(method, declaration);
        }

        endScope();

        // once done w/ resolving class's methods...
        // ...the scope is discarded
        if (stmt.superclass != null)
            endScope();
        
        currentClass = enclosingClass;

        return null;
    }

    @Override
    public Void visitExpressionStmt(Stmt.Expression stmt) {
        resolve(stmt.expression);

        return null;
    }

    @Override
    public Void visitFunctionStmt(Stmt.Function stmt) {
        declare(stmt.name);
        define(stmt.name);

        resolveFunction(stmt, FunctionType.FUNCTION);

        return null;
    }
    
    // static analysis, bc we are analyzing any branch...
    // ... that can run so we have to resolve both branches
    @Override
    public Void visitIfStmt(Stmt.If stmt) {
        resolve(stmt.condition);
        resolve(stmt.thenBranch);

        if (stmt.elseBranch != null)
            resolve(stmt.elseBranch);

        return null;
    }

    @Override
    public Void visitPrintStmt(Stmt.Print stmt) {
        resolve(stmt.expression);

        return null;
    }

    @Override
    public Void visitReturnStmt(Stmt.Return stmt) {
        if (currentFunction == FunctionType.NONE)
            Lox.error(stmt.keyword, "can't return from top-level code");
        
        if (stmt.value != null) {
            if (currentFunction == FunctionType.INITIALIZER)
                Lox.error(stmt.keyword, "can't return a value from an initializer");
            
            resolve(stmt.value);
        }
        
        return null;
    }

    // binding is done in two steps...
    // ...in order to handle funny edge cases
    @Override
    public Void visitVarStmt(Stmt.Var stmt) {
        // ...(1) declaring
        declare(stmt.name);

        if (stmt.initializer != null)
            resolve(stmt.initializer);
        
        // ...(2) defining
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
    public Void visitGetExpr(Expr.Get expr) {
        resolve(expr.object);
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
    public Void visitTernaryExpr(Expr.Ternary expr) {
        resolve(expr.left);
        resolve(expr.mid);
        resolve(expr.right);

        return null;
    }

    @Override
    public Void visitSetExpr(Expr.Set expr) {
        resolve(expr.value);
        resolve(expr.object);
        return null;
    }

    @Override
    public Void visitSuperExpr(Expr.Super expr) {
        if (currentClass == ClassType.NONE)
            Lox.error(expr.keyword, 
                "can't use 'super' outside of a class");
        else if (currentClass != ClassType.SUBCLASS)
            Lox.error(expr.keyword, 
                "can't use 'super' in a class w/ no superclass");
        
        // resolve the super token exactly...
        // ... as if it were a variable
        resolveLocal(expr, expr.keyword);

        return null;
    }

    @Override
    public Void visitThisExpr(Expr.This expr) {
        if (currentClass == ClassType.NONE) {
            Lox.error(expr.keyword, 
                "can't use 'this' outside of a class");
            return null;
        }
        resolveLocal(expr, expr.keyword);

        return null;
    }

    @Override
    public Void visitUnaryExpr(Expr.Unary expr) {
        resolve(expr.right);
        
        return null;
    }

    @Override
    public Void visitVariableExpr(Expr.Variable expr) {
        // variable exists in the current scope...
        // ... but value is false (declared and not defined)
        boolean not_defined = !scopes.isEmpty() &&
                            scopes.peek().get(expr.name.lexeme) == Boolean.FALSE;
        
        if (not_defined)
            Lox.error(expr.name, "can't read local variable in its own initializer");
        
        // resolve the variable itself using this helper
        resolveLocal(expr, expr.name);
        
        return null;
    }

    // adds var to the innermost scope ...
    // ... so that it shadows any outer one ...
    // ... and so that we know the var exists
    private void declare(Token name) {
        if (scopes.isEmpty())
            return;
        
        Map<String, Boolean> scope = scopes.peek();
        if (scope.containsKey(name.lexeme))
            Lox.error(name, "a variable with this name already exists in this scope");

        // marking variable as not ready yet by...
        // ...binding its name to false in the scope map
        scope.put(name.lexeme, false);
    }

    private void define(Token name) {
        if (scopes.isEmpty())
            return;
        
        // marked as fully initialized and avail for use
        scopes.peek().put(name.lexeme, true);
    }
    
    void resolve(List<Stmt> stmts) {
        for (Stmt stmt : stmts)
            resolve(stmt);
    }

    private void resolve(Stmt stmt) {
        stmt.accept(this);
    }

    private void resolve(Expr expr) {
        expr.accept(this);
    }

    private void resolveFunction(Stmt.Function function, FunctionType type) {
        FunctionType enclosingFunction = currentFunction;
        currentFunction = type;

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
        currentFunction = enclosingFunction;
    }

    private void resolveLocal(Expr expr, Token name) {
        // start at the innermost scope and work outwards
        for (int i = scopes.size() - 1; i >= 0; i--) {
            // look in each map for a matching name
            if (scopes.get(i).containsKey(name.lexeme)) {
                interpreter.resolve(expr, scopes.size() - 1 - i);
                return;
            }
        }
    }
}