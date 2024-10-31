CC = clang
CC_CHECK_FLAGS = -fsyntax-only

LOX_DIR = ./com/craftinginterpreters/lox
TOOL_DIR = ./com/craftinginterpreters/tool

LOX_FILE_DIR = ../lox-files/
LOX_FILE ?=

.PHONY: clox
clox: check-clox compile-clox run-clox

.PHONY: jlox
jlox: compile-jlox run-jlox

.PHONY: jlox-tool
jlox-tool: compile-jlox-tool run-jlox-tool

.PHONY: run-clox
run-clox:
	./c/main

.PHONY: run-jlox
run-jlox:
	cd ./java && java com.craftinginterpreters.lox.Lox $(LOX_FILE)

# .PHONY: run-lox-file
# run-lox-file:
# 	cd ./java && java com.craftinginterpreters.lox.Lox $(LOX_FILE_DIR) $(LOX_FILE)

.PHONY: run-jlox-tool
run-jlox-tool:
	cd ./java && java com.craftinginterpreters.tool.GenerateAst $(LOX_DIR)

.PHONY: compile-clox
compile-clox:
	cd ./c && $(CC) -o main main.c

.PHONY: compile-jlox-tool
compile-jlox-tool:
	cd ./java && javac $(TOOL_DIR)/*.java

.PHONY: compile-jlox
compile-jlox:
	cd ./java && javac $(LOX_DIR)/*.java

.PHONY: check-clox
check-clox:
	cd ./c && $(CC) chunk.c $(CC_CHECK_FLAGS)
	cd ./c && $(CC) main.c $(CC_CHECK_FLAGS)

.PHONY: clean-clox
clean-clox:
	rm -f ./c/main

.PHONY: clean-jlox-tool
clean-jlox-tool: clean-jlox-tool clean-lox

.PHONY: clean-jlox
clean-jlox:
	rm -f java/$(LOX_DIR)/*.class

.PHONY: clean-jlox-tool
clean-jlox-tool:
	rm -f java/$(TOOL_DIR)/*.class