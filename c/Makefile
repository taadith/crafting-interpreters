CC = clang
CC_CHECK_FLAGS = -fsyntax-only -Wall -Wextra -Wpedantic
CC_COMPILE_FLAGS = -fsanitize=address

C_FILES = chunk.c main.c

.PHONY: clox
clox: check-clox compile-clox run-clox

.PHONY: run-clox
run-clox:
	./main

.PHONY: compile-clox
compile-clox:
	$(CC) -o main $(C_FILES) $(CC_COMPILE_FLAGS)

.PHONY: check-clox
check-clox:
	$(CC) $(C_FILES) $(CC_CHECK_FLAGS)

.PHONY: clean
clean:
	rm -f ./c/main
