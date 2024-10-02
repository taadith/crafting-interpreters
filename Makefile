LOX_DIR = ./com/craftinginterpreters/lox
TOOL_DIR = ./com/craftinginterpreters/tool
LOX_FILE ?=

.PHONY: all
all: tool run

.PHONY: ast-printer
ast-printer: tool compile-lox
	cd ./java && java com.craftinginterpreters.lox.AstPrinter

.PHONY: lox
lox: compile-lox run-lox

.PHONY: tool
tool: compile-tool run-tool

.PHONY: run-lox
run-lox:
	cd ./java && java com.craftinginterpreters.lox.Lox $(LOX_FILE)

.PHONY: run-tool
run-tool:
	cd ./java && java com.craftinginterpreters.tool.GenerateAst $(LOX_DIR)

.PHONY: compile-tool
compile-tool:
	cd ./java && javac $(TOOL_DIR)/*.java

.PHONY: compile-lox
compile-lox:
	cd ./java && javac $(LOX_DIR)/*.java

.PHONY: clean
clean: clean-tool clean-lox

.PHONY: clean-lox
clean-lox:
	rm -f java/$(LOX_DIR)/*.class

.PHONY: clean-tool
clean-tool:
	rm -f java/$(LOX_DIR)/Expr.java
	rm -f java/$(TOOL_DIR)/*.class