LOX_DIR = com/craftinginterpreters/lox
TOOL_DIR = com/craftinginterpreters/tool

all: clean-lox compile run

run: compile
	cd ./java && java com.craftinginterpreters.lox.Lox

run-file: compile
	cd ./java && java com.craftinginterpreters.lox.Lox file.lox

compile:
	cd ./java && javac $(LOX_DIR)/*.java

clean: clean-lox clean-tool

clean-lox:
	rm java/$(LOX_DIR)/*.class

clean-tool:
	rm java/$(TOOL_DIR)/*.class