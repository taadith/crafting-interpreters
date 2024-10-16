package com.craftinginterpreters.lox;

import java.util.List;

class LoxFunction implements LoxCallable {
    private final Stmt.Function declaration;
    
    LoxFunction(Stmt.Function declaration) {
        this.declaration = declaration;
    }

    @Override
    public String toString() {
        return "<fn " + declaration.name.lexeme + ">";
    }

    @Override
    public int arity() {
        return declaration.params.size();
    }

    @Override
    public Object call(Interpreter interpreter,
                       List<Object> arguments) {

        // each function call gets its own environment
        Environment env = new Environment(interpreter.globals);

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

        return null;
    }
}
