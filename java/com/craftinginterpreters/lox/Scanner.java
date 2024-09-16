package java.com.craftinginterpreters.lox;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import java.com.craftinginterpreters.TokenType.*;

class Scanner {
    private final String src;
    private final List<Token> tokens = new ArrayList<>();

    Scanner(String src) {
        this.src = src;
    }
}
