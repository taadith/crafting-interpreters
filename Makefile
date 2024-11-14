CC = clang
CC_CHECK_FLAGS = -fsyntax-only -Wall -Wextra -Wpedantic
CC_COMPILE_FLAGS = -fsanitize=address

C_FILES = chunk.c compiler.c debug.c main.c memory.c object.c rle.c scanner.c value.c vm.c

JLOX_DIR = 		./com/craftinginterpreters/lox
JLOX_TOOL_DIR = ./com/craftinginterpreters/tool

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

.PHONY: run-jlox-tool
run-jlox-tool:
	cd ./java && java com.craftinginterpreters.tool.GenerateAst $(JLOX_DIR)

.PHONY: compile-clox
compile-clox:
	cd ./c && $(CC) -o main $(C_FILES) $(CC_COMPILE_FLAGS)

.PHONY: compile-jlox-tool
compile-jlox-tool:
	cd ./java && javac $(JLOX_TOOL_DIR)/*.java

.PHONY: compile-jlox
compile-jlox:
	cd ./java && javac $(JLOX_DIR)/*.java

.PHONY: check-clox
check-clox:
	cd ./c && $(CC) $(C_FILES) $(CC_CHECK_FLAGS)

.PHONY: clean
clean: clean-tool clean-jlox clean-clox

.PHONY: clean-clox
clean-clox:
	rm -f ./c/main

.PHONY: clean-jlox
clean-jlox:
	rm -f java/$(JLOX_DIR)/*.class

.PHONY: clean-tool
clean-tool:
	rm -f java/$(JLOX_TOOL_DIR)/*.class