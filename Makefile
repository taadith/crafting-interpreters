LOX_DIR = com/craftinginterpreters/lox
TOOL_DIR = com/craftinginterpreters/tool
LOX_FILE = file.lox

all-repl: clean-lox compile run-repl

all-file: clean-lox compile run-file

run-repl: compile
	cd ./java && java com.craftinginterpreters.lox.Lox

run-file: compile
	cd ./java && java com.craftinginterpreters.lox.Lox $(LOX_FILE)

compile:
	cd ./java && javac $(LOX_DIR)/*.java

clean: clean-lox clean-tool

clean-lox:
	rm java/$(LOX_DIR)/*.class

clean-tool:
	rm java/$(TOOL_DIR)/*.class