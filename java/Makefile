JLOX_DIR = ./com/craftinginterpreters/lox
JLOX_TOOL_DIR = ./com/craftinginterpreters/tool

LOX_FILE ?=

.PHONY: jlox
jlox: compile-jlox run-jlox

.PHONY: jlox-tool
jlox-tool: compile-jlox-tool run-jlox-tool

.PHONY: run-jlox
run-jlox:
	java com.craftinginterpreters.lox.Lox $(LOX_FILE)

.PHONY: run-jlox-tool
run-jlox-tool:
	java com.craftinginterpreters.tool.GenerateAst $(JLOX_DIR)

.PHONY: compile-jlox-tool
compile-jlox-tool:
	javac $(JLOX_TOOL_DIR)/*.java

.PHONY: compile-jlox
compile-jlox:
	javac $(JLOX_DIR)/*.java

.PHONY: clean
clean: clean-tool clean-jlox

.PHONY: clean-jlox
clean-jlox:
	rm -f $(JLOX_DIR)/*.class

.PHONY: clean-tool
clean-tool:
	rm -f $(JLOX_TOOL_DIR)/*.class
