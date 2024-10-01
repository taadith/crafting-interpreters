LOX_DIR = ./com/craftinginterpreters/lox
TOOL_DIR = ./com/craftinginterpreters/tool
LOX_FILE ?=

all: clean compile run

run: run-tool run-lox

run-lox: clean-lox compile-lox
	cd ./java && java com.craftinginterpreters.lox.Lox $(LOX_FILE)

run-tool: clean-tool compile-tool
	cd ./java && java com.craftinginterpreters.tool.GenerateAst $(LOX_DIR)

compile: compile-tool compile-lox

compile-tool:
	cd ./java && javac $(TOOL_DIR)/*.java

compile-lox:
	cd ./java && javac $(LOX_DIR)/*.java

clean: clean-tool clean-lox

clean-lox:
	rm -f java/$(LOX_DIR)/*.class
	
clean-tool:
	rm -f java/$(LOX_DIR)/Expr.java
	rm -f java/$(TOOL_DIR)/*.class

.PHONY: all run compile clean clean-lox clean-tool