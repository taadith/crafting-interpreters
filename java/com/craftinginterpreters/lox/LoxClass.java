package com.craftinginterpreters.lox;

import java.util.List;
import java.util.Map;

class LoxClass implements LoxCallable{
    final String name;
    final LoxClass superclass;
    private final Map<String, LoxFunction> methods;

    LoxClass(String name, LoxClass superclass, Map<String, LoxFunction> methods) {
        this.name = name;
        this.superclass = superclass;
        this.methods = methods;
    }

    LoxFunction findMethod(String name) {
        if (methods.containsKey(name))
            return methods.get(name);

        // recurse up the superclass chain...
        // ... to loop for the method def
        if (superclass != null)
            return superclass.findMethod(name);
        
        return null;
    }

    // calling a class instantiates a new LoxInstance...
    // ...for the called class and returns it
    @Override
    public Object call(Interpreter interpreter, List<Object> args) {
        LoxInstance instance = new LoxInstance(this);

        // calling a class (after creating the LoxInstance)...
        // ... we look for an "init" method
        LoxFunction initializer = findMethod("init");

        // if we find an "init" method, we immediately...
        // ... bind and invoke it just like a method call
        if (initializer != null)
            initializer.bind(instance).call(interpreter, args);
        
        return instance;
    }

    @Override
    public int arity() {
        LoxFunction initializer = findMethod("init");
        
        if (initializer == null)
            return 0;
        return initializer.arity();
    }

    @Override
    public String toString() {
        return name;
    }
}
