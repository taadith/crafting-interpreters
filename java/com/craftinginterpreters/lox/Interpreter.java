package com.craftinginterpreters.lox;

import java.util.List;

// declaring itself as a visitor...
// ... also Void is a generic type arg
class Interpreter implements Expr.Visitor<Object>,
                             Stmt.Visitor<Void> {
    
    private Environment env = new Environment();

    public void interpret(List<Stmt> stmts) {
        try {
            for (Stmt stmt : stmts)
                execute(stmt);
            
        } catch(RuntimeError error) {
            Lox.runtimeError(error);
        }
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
    public Void visitExpressionStmt(Stmt.Expression stmt) {
        evaluate(stmt.expression);
        return null;
    }

    @Override
    public Void visitPrintStmt(Stmt.Print stmt) {
        Object value = evaluate(stmt.expression);
        System.out.println(stringify(value));
        return null;
    }

    @Override
    public Void visitBlockStmt(Stmt.Block stmt) {
        executeBlock(stmt.statements, new Environment(env));
        return null;
    }

    // evalutes rhs to get the value and store it in a named variable
    @Override
    public Object visitAssignExpr(Expr.Assign expr) {
        Object value = evaluate(expr.value);
        env.assign(expr.name, value);
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

    private boolean isEqual(Object a, Object b) {
        if (a == null && b == null)
            return true;
        if (a == null)
            return false;
        
        return a.equals(b);
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
    // ... the heavy lifting of determine if the variable is defined
    @Override
    public Object visitVariableExpr(Expr.Variable expr) {
        return env.get(expr.name);
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

}