package com.craftinginterpreters.lox;

import java.util.List;

class LoxFunction implements LoxCallable {
    private final Stmt.Function declaration;
    private final Environment closure;
    private final boolean isInitializer;
    
    LoxFunction(Stmt.Function declaration, Environment closure,
                boolean isInitializer) {
        this.isInitializer = isInitializer;
        this.declaration = declaration;
        this.closure = closure;
    }

    @Override
    public int arity() {
        return declaration.params.size();
    }

    @Override
    public Object call(Interpreter interpreter,
                       List<Object> arguments) {

        // each function call gets its own environment
        Environment env = new Environment(closure);

        // define each param and bind arg value to it
        for(int i = 0; i < declaration.params.size(); i++)
            env.define(declaration.params.get(i).lexeme, arguments.get(i));
        
        // discards function-local env and restores the...
        // ... previous one that was active back at the callsite
        try {
            interpreter.executeBlock(declaration.body, env);
        } catch (Return returnValue) {
            return returnValue.value;
        }

        // if the function is an initializer...
        // ... we override the actual return value and forcibly return "this"
        if (isInitializer)
            return closure.getAt(0, "this");

        return null;
    }

    @Override
    public String toString() {
        return "<fn " + declaration.name.lexeme + ">";
    }

    LoxFunction bind(LoxInstance instance) {
        // new env nested inside the method's original closure...
        // ...sort of a closure w/in a closure
        Environment env = new Environment(closure);

        // declare "this" as a var in that env and bind it to...
        // ...given instance
        env.define("this", instance);
        return new LoxFunction(declaration, env, isInitializer);
    }
}
