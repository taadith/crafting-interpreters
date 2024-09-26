package com.craftinginterpreters.lox;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.charset.Charset;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.List;

public class Lox {
    static boolean hadError = false;

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
    }

    // fires up an interactive prompt or REPL (read, eval, print, loop)
    // REPL comes from Lisp -> (print (eval (read)))
    private static void runPrompt() throws IOException {
        InputStreamReader input = new InputStreamReader(System.in);
        BufferedReader reader = new BufferedReader(input);

        for(;;) {
            System.out.print("> ");

            // killing an interactice command-line app w/ Ctrl-D...
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

        // for now, just print the tokens
        // we will change later
        for (Token token : tokens)
            System.out.println(token);
    }

    static void error(int line, String msg) {
        report(line, "", msg);
    }

    private static void report(int line, String where, String msg) {
        System.err.println("[line " + line + "] Error" +
            where + ": " + msg);
        hadError = true;
    }
}