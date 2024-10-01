package com.craftinginterpreters.tool;

import java.io.IOException;
import java.io.PrintWriter;
import java.util.Arrays;
import java.util.List;

public class GenerateAst {
    public static void main(String[] args) throws IOException {
        if (args.length != 1) {
            System.err.println("usage: generate_ast <output directory>");
            
            // command used incorrectly (UNIX "sysexits.h" header)
            System.exit(64);
        }

        String outputDir = args[0];

        defineAst(outputDir, "Expr", Arrays.asList(
            "Binary    : Expr left, Token operator, Expr right",
            "Grouping       : Expr expression",
            "Literal        : Object value",
            "Unary          : Token operator, Expr right"
        ));
    }

    private static void defineAst(
            String outputDir, String baseName, List<String> types) 
            throws IOException {
        
        String path = outputDir + "/" + baseName + ".java";
        PrintWriter writer = new PrintWriter(path, "UTF-8");
        
        // package header
        writer.println("package com.craftinginterpreters.lox;");
        writer.println();
        
        // imports
        writer.println("import java.util.List;");
        writer.println();
        
        // body of file
        writer.println("abstract class " + baseName + " {");

        // the AST classes
        for (String type: types) {
            // split class name and fields belonging to class...
            // ... from defineAst()'s param List<String> types
            String className = type.split(":")[0].trim();
            String fields = type.split(":")[1].trim();
            
            // 
            defineType(writer, baseName, className, fields);
        }
        writer.println("}");

        // closing file
        writer.close();
    }

    private static void defineType(
            PrintWriter writer, String baseName,
            String className, String fieldList) {
        
        writer.println("\t static class " + className + " extends " +
            baseName + " {");
        
        // constructor
        writer.println("\t" + className + "(" + fieldList + ") {");

        // store params in fields
        String[] fields = fieldList.split(", ");
        for(String field : fields) {
            String name = field.split(" ")[1];
            writer.println("\t\tthis." + name + " = " + name + ";");
        }
        writer.println("\t\t}");

        // fields
        writer.println();
        for (String field : fields)
            writer.println("\tfinal " + field + ";");
        
        writer.println("\t}");
    }
}
