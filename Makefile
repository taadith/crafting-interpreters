compile:
	cd ./java && javac com/craftinginterpreters/lox/*.java

run:
	cd ./java && java com.craftinginterpreters.lox.Lox

clean:
	rm java/com/craftinginterpreters/lox/*.class