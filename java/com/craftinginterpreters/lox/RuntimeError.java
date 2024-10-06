package com.craftinginterpreters.lox;

// tracks token that IDs where runtime error came from
class RuntimeError extends RuntimeException {
    final Token token;

    RuntimeError(Token token, String msg) {
        super(msg);
        this.token = token;
    }
}
