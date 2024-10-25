package com.craftinginterpreters.lox;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

// declaring itself as a visitor...
// ... also Void is a generic type arg
class Interpreter implements Expr.Visitor<Object>,
                             Stmt.Visitor<Void> {
    
    final Environment globals = new Environment();
    private Environment env = globals;
    private final Map<Expr, Integer> locals = new HashMap<>();

    Interpreter() {
        globals.define("clock", new LoxCallable() {
            @Override
            public int arity() {
                return 0;
            }

            @Override
            public Object call(Interpreter interpreter,
                               List<Object> args) {
                return (double)System.currentTimeMillis() / 1000.0;
            }

            @Override
            public String toString() {
                return "<navtive fn>";
            }
        });
    }

    public void interpret(List<Stmt> stmts) {
        try {
            for (Stmt stmt : stmts)
                execute(stmt);
        }
        catch(RuntimeError error) {
            Lox.runtimeError(error);
        }
    }

    @Override
    public Void visitBlockStmt(Stmt.Block stmt) {
        executeBlock(stmt.statements, new Environment(env));
        return null;
    }

    @Override
    public Void visitClassStmt(Stmt.Class stmt) {
        // declaring class's name in current env
        env.define(stmt.name.lexeme, null);

        // turning method declarations into a LoxFunction object (runtime representation)
        Map<String, LoxFunction> methods = new HashMap<>();
        for (Stmt.Function method : stmt.methods) {
            LoxFunction function = new LoxFunction(method, env);
            methods.put(method.name.lexeme, function);
        }

        // turn class syntax node into a LoxClass...
        // ... (the runtime representation of a class)
        LoxClass klass = new LoxClass(stmt.name.lexeme, methods);

        // store class object in previously defined variable
        env.assign(stmt.name, klass);

        return null;
    }

    @Override
    public Void visitExpressionStmt(Stmt.Expression stmt) {
        evaluate(stmt.expression);
        return null;
    }

    @Override
    public Void visitFunctionStmt(Stmt.Function stmt) {
        // capture the env in which function is declared, not called
        LoxFunction function = new LoxFunction(stmt, env);

        env.define(stmt.name.lexeme, function);
        return null;
    }

    @Override
    public Void visitIfStmt(Stmt.If stmt) {
        if (isTruthy(evaluate(stmt.condition)))
            execute(stmt.thenBranch);
        else if (stmt.elseBranch != null)
            execute(stmt.elseBranch);
        return null;
    }

    @Override
    public Void visitPrintStmt(Stmt.Print stmt) {
        Object value = evaluate(stmt.expression);
        System.out.println(stringify(value));
        return null;
    }

    @Override
    public Void visitReturnStmt(Stmt.Return stmt) {
        Object value = null;
        if (stmt.value != null)
            value = evaluate(stmt.value);
        
        throw new Return(value);
    }

    @Override
    public Void visitVarStmt(Stmt.Var stmt) {
        Object value = null;

        // evaluate variable w/ initalizer
        if (stmt.initializer != null)
            value = evaluate(stmt.initializer);
        
        // set it to nil if it comes w/ no initializer
        env.define(stmt.name.lexeme, value);

        return null;
    }

    @Override
    public Void visitWhileStmt(Stmt.While stmt) {
        while (isTruthy(evaluate(stmt.condition)))
            execute(stmt.body);
        
        return null;
    }

    // evalutes rhs to get the value and store it in a named variable
    @Override
    public Object visitAssignExpr(Expr.Assign expr) {
        Object value = evaluate(expr.value);
        
        Integer distance = locals.get(expr);
        if (distance != null)
            env.assignAt(distance, expr.name, value);
        else
            globals.assign(expr.name, value);

        return value;
    }

    @Override
    public Object visitBinaryExpr(Expr.Binary expr) {
        Object left = evaluate(expr.left);
        Object right = evaluate(expr.right);

        switch(expr.operator.type) {
            // "expression" (?) operator
            case COMMA:
                return right;

            // equality operators
            case BANG_EQUAL:
                return !isEqual(left, right);

            case EQUAL_EQUAL:
                return isEqual(left, right);


            // comparison operators:
            case GREATER:
                checkNumberOperands(expr.operator, left, right);
                return (double)left > (double)right;

            case GREATER_EQUAL:
                checkNumberOperands(expr.operator, left, right);
                return (double)left >= (double)right;

            case LESS:
            checkNumberOperands(expr.operator, left, right);
                return (double)left < (double)right;

            case LESS_EQUAL:
            checkNumberOperands(expr.operator, left, right);
                return (double)left <= (double)right;

            // arithmetic operators:
            case MINUS:
                checkNumberOperands(expr.operator, left, right);
                return (double)left - (double)right;

            case PLUS:
                // one or the other is a String
                if (left instanceof String && right instanceof Double)
                    return (String)left + stringify(right);
                if (left instanceof Double && right instanceof String)
                    return stringify(left) + (String)right;
                // both are the same type
                if (left instanceof Double && right instanceof Double)
                    return (double)left + (double)right;
                if (left instanceof String && right instanceof String)
                    return (String)left + (String)right;
                throw new RuntimeError(expr.operator,
                    "operands must numbers or strings");
            
            case SLASH:
                checkNumberOperands(expr.operator, left, right);
                if ((double)right == 0)
                    throw new RuntimeError(expr.operator,
                        "divisor (right operand) must be non-zero");
                return (double)left / (double)right;
            
            case STAR:
                checkNumberOperands(expr.operator, left, right);
                return (double)left * (double)right;
        }

        // unreachable
        return null;
    }

    private boolean isEqual(Object a, Object b) {
        if (a == null && b == null)
            return true;
        if (a == null)
            return false;
        
        return a.equals(b);
    }
    
    @Override
    public Object visitCallExpr(Expr.Call expr) {
        Object callee = evaluate(expr.callee);

        List<Object> args = new ArrayList<>();
        for (Expr arg : expr.args)
            args.add(evaluate(arg));
        
        if (!(callee instanceof LoxCallable)) {
            throw new RuntimeError(expr.paren, 
                "can only call functions and classes");
        }
        
        LoxCallable function = (LoxCallable) callee;
        if (args.size() != function.arity()) {
            throw new RuntimeError(expr.paren, "expected " +
                function.arity() + " arguments but got " +
                args.size());
        }

        return function.call(this, args);
    }
    
    @Override
    public Object visitGetExpr(Expr.Get expr) {
        // evaluate the expression whose property is being accessed
        Object obj = evaluate(expr.object);

        // look up the property
        if (obj instanceof LoxInstance)
            return ((LoxInstance) obj).get(expr.name);
        
        throw new RuntimeError(expr.name, "only instances have properties");
    }

    // grouping node has a reference to an inner node...
    // ... for the expression contained w/in the "()"
    @Override
    public Object visitGroupingExpr(Expr.Grouping expr) {
        return evaluate(expr.expression);
    }

    // converting the literal tree node...
    // ... into a runtime value is trivial
    @Override
    public Object visitLiteralExpr(Expr.Literal expr) {
        return expr.value;
    }

    @Override
    public Object visitLogicalExpr(Expr.Logical expr) {
        Object left = evaluate(expr.left);

        if (expr.operator.type == TokenType.OR) {
            if (isTruthy(left))
                return left;
        }
        else {
            if (!isTruthy(left))
                return left;
        }

        return evaluate(expr.right);
    }

    @Override
    public Object visitSetExpr(Expr.Set expr) {
        Object obj = evaluate(expr.object);

        // error if obj isn't LoxInstance
        if (!(obj instanceof LoxInstance))
            throw new RuntimeError(expr.name, "only instances have fields");
        
        // evaluate the value being set...
        Object value = evaluate(expr.value);

        // ...and store it on the instance
        ((LoxInstance)obj).set(expr.name, value);

        return value;
    }

    @Override
    public Object visitTernaryExpr(Expr.Ternary expr) {
        Object left = evaluate(expr.left);
        Object mid = evaluate(expr.mid);
        Object right = evaluate(expr.right);

        if (expr.op1.type == TokenType.QUESTION && expr.op2.type == TokenType.COLON) {
            if(isTruthy(left))
                return mid;
            return right;
        }

        // unreachable (?)
        return null;
    }
    
    @Override
    public Object visitThisExpr(Expr.This expr) {
        return lookUpVariable(expr.keyword, expr);
    }
    
    @Override
    public Object visitUnaryExpr(Expr.Unary expr) {
        // bc unary expressions are right associative...
        // ... we first evaluate the operand expression
        Object right = evaluate(expr.right);

        switch (expr.operator.type) {
            case BANG:
                return !isTruthy(right);
            case MINUS:
                checkNumberOperand(expr.operator, right);
                // type cast before performing the operation...
                // ... at runtime before "-" is evaluated ...
                // ... making this why language is dynamically typed
                return -(double)right;
        }

        // unreachable
        return null;
    }

    // forwards to the environment, which does...
    // ... the heavy lifting of determining if the variable is defined
    @Override
    public Object visitVariableExpr(Expr.Variable expr) {
        return lookUpVariable(expr.name, expr);
    }

    void executeBlock(List<Stmt> stmts,
                      Environment env) {
        
        Environment previous = this.env;
        try {
            this.env = env;

            // executing list of Stmt in the context of a given env
            for (Stmt stmt : stmts)
                execute(stmt);
        } finally {
            this.env = previous;
        }
    }
    
    private void execute(Stmt stmt) {
        stmt.accept(this);
    }
    
    private Object evaluate(Expr expr) {
        return expr.accept(this);
    }

    void resolve(Expr expr, int depth) {
        locals.put(expr, depth);
    }

    private boolean isTruthy(Object obj) {
        if (obj == null)
            return false;
        if (obj instanceof Boolean)
            return (boolean)obj;
        return true;
    }

    private void checkNumberOperand(Token operator, Object operand) {
        if (operand instanceof Double)
            return;

        throw new RuntimeError(operator, "operand must be a number");
    }

    private void checkNumberOperands(Token op, Object left, Object right) {
        if (left instanceof Double && right instanceof Double)
            return;

        throw new RuntimeError(op, "operands must be numbers");
    }

    private String stringify(Object obj) {
        if (obj == null)
            return "nil";
        
        if (obj instanceof Double) {
            String txt = obj.toString();
            if (txt.endsWith(".0"))
                txt = txt.substring(0, txt.length() - 2);
            return txt;
        }

        return obj.toString();
    }

    private Object lookUpVariable(Token name, Expr expr) {
        Integer distance = locals.get(expr);
        if (distance != null)
            return env.getAt(distance, name.lexeme);
        else
            return globals.get(name);
    }
}