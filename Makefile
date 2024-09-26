LOX_DIR = com/craftinginterpreters/lox
TOOL_DIR = com/craftinginterpreters/tool
LOX_FILE ?=

run: clean-lox compile
	cd ./java && java com.craftinginterpreters.lox.Lox $(LOX_FILE)

compile:
	cd ./java && javac $(LOX_DIR)/*.java

# will eventually delete clean-lox and clean-tool & just consolidate the commands into clean
clean: clean-lox clean-tool

# temp command
clean-lox:
	rm -f java/$(LOX_DIR)/*.class

# temp command
clean-tool:
	rm -f java/$(TOOL_DIR)/*.class

.PHONY: run compile clean clean-lox clean-tool