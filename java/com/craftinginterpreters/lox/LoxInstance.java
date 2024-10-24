// runtime representation of an instance of a Lox class

package com.craftinginterpreters.lox;

import java.util.HashMap;
import java.util.Map;

class LoxInstance {
    private LoxClass klass;

    // each key in the map is a property name and the...
    // ...corresponding value is the property's value
    private final Map<String, Object> fields = new HashMap<>();

    LoxInstance(LoxClass klass) {
        this.klass = klass;
    }

    Object get(Token name) {
        if (fields.containsKey(name.lexeme))
            return fields.get(name.lexeme);

        throw new RuntimeError(name,
            "undefined property '" + name.lexeme + "'");
    }

    @Override
    public String toString() {
        return klass.name + " instance";
    }
}
