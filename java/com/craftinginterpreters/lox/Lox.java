package com.craftinginterpreters.lox;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.charset.Charset;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.List;


public class Lox {
    private static final Interpreter interpreter = new Interpreter();
    static boolean hadError = false;
    static boolean hadRuntimeError = false;

    public static void main(String[] args) throws IOException {
        if (args.length > 1) {
            System.out.println("usage: jlox [script]");

            // command used incorrectly (UNIX "sysexits.h" header)
            System.exit(64);
        }
        else if (args.length == 1)
            runFile(args[0]);
        else
            runPrompt();
    }

    // reads file and executes
    private static void runFile(String path) throws IOException {
        byte[] bytes = Files.readAllBytes(Paths.get(path));
        run(new String(bytes, Charset.defaultCharset()));

        // input data is incorrect (UNIX "sysexits.h" header)
        if (hadError)
            System.exit(65);
        
        // internal software error (UNIX "sysexits.h" header)
        if (hadRuntimeError)
            System.exit(70);
    }

    // fires up an interactive prompt or REPL (read, eval, print, loop)
    // REPL comes from Lisp -> (print (eval (read)))
    private static void runPrompt() throws IOException {
        InputStreamReader input = new InputStreamReader(System.in);
        BufferedReader reader = new BufferedReader(input);

        for(;;) {
            System.out.print("> ");

            // killing an interactive command-line app w/ Ctrl-D...
            // ... signalling an EOF condition to the program, ...
            // ... subsequently, readLine() returns null
            String line = reader.readLine();
            if(line == null)
                break;
            
            run(line);

            // resetting flag in interactive loop
            // just report the error, don't stop REPL
            hadError = false;
        }
    }

    private static void run(String src) {
        Scanner scanner = new Scanner(src);
        List<Token> tokens = scanner.scanTokens();
        Parser parser = new Parser(tokens);

        
        List<Stmt> stmts = parser.parse();

        // stop if there's a syntax error
        if (hadError)
            return;
        
        interpreter.interpret(stmts);
    }

    static void error(int line, String msg) {
        report(line, "", msg);
    }

    private static void report(int line, String where, String msg) {
        System.err.println("[line " + line + "] error" +
            where + ": " + msg);
        hadError = true;
    }

    // reports error @ a given token
    static void error(Token token, String msg) {
        if (token.type == TokenType.EOF)
            report(token.line, " at end", msg);
        else
            report(token.line, " at '" + token.lexeme + "'", msg);
    }

    static void runtimeError(RuntimeError error) {
        // token associated w/ error tells user...
        // ... the line of code executing when...
        // ... the error occurred
        System.err.println(error.getMessage() +
            "\n[line " + error.token.line + "]");
        
        hadRuntimeError = true;
    }
}