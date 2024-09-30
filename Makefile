LOX_DIR = com/craftinginterpreters/lox
TOOL_DIR = com/craftinginterpreters/tool
LOX_FILE ?=

all: clean compile run

run: compile
	cd ./java && java com.craftinginterpreters.tool.GenerateAst ../lox/
	cd ./java && java com.craftinginterpreters.lox.Lox $(LOX_FILE)

compile: clean
	cd ./java && javac $(TOOL_DIR)/*.java
	cd ./java && javac $(LOX_DIR)/*.java

run-lox: compile-lox
	cd ./java && java com.craftinginterpreters.lox.Lox $(LOX_FILE)

compile-lox: clean-lox
	cd ./java && javac $(LOX_DIR)/*.java

run-tool: compile-tool
	cd ./java && java com.craftinginterpreters.tool.GenerateAst ../lox/

compile-tool: clean-tool
	cd ./java && javac $(TOOL_DIR)/*.java

# will eventually delete clean-lox and clean-tool & just consolidate the commands into clean
clean: clean-lox clean-tool

# temp command
clean-lox:
	rm -f java/$(LOX_DIR)/*.class

# temp command
clean-tool:
	rm -f java/$(TOOL_DIR)/*.class

.PHONY: all run compile clean clean-lox clean-tool