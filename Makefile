LOX_DIR = com/craftinginterpreters/lox
TOOL_DIR = com/craftinginterpreters/tool
LOX_FILE ?=

run: clean-lox compile
	cd ./java && java com.craftinginterpreters.lox.Lox $(LOX_FILE)

compile:
	cd ./java && javac $(LOX_DIR)/*.java

clean: clean-lox clean-tool

clean-lox:
	rm -f java/$(LOX_DIR)/*.class

clean-tool:
	rm -f java/$(TOOL_DIR)/*.class

.PHONY: run compile clean clean-lox clean-tool