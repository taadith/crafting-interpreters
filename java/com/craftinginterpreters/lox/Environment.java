package com.craftinginterpreters.lox;

import java.util.HashMap;
import java.util.Map;

class Environment {
    private final Map<String, Object> values = new HashMap<>();

    // variable definitions bind a new name to a value
    void define(String name, Object value) {
        // when we add key to the map, we don't check to see if it's already present
        values.put(name, value);
    }

    // looking up an existing variable
    Object get(Token name) {
        // if variable is found, return the value bound to it
        if (values.containsKey(name.lexeme))
            return values.get(name.lexeme);
        
        // important choice to make it a runtime error, not a syntax error or to simply allow for it
        throw new RuntimeError(name,
            "undefined variable '" + name.lexeme + "'");
    }

    // not allowed to create a new variable
    void assign(Token name, Object value) {
        if (values.containsKey(name.lexeme)) {
            values.put(name.lexeme, value);
            return;
        }

        // runtime error if key doesn't already exist in env's variable map
        throw new RuntimeError(name,
            "undefined variable '" + name.lexeme + "'");
    }
}
