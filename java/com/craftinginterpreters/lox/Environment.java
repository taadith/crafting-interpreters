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
}
