CC := clang
C_FLAGS1 := -Wall -Wextra -Wpedantic -g -c
C_FLAGS2 := -g

OBJ_FILES := chunk.o debug.o main.o memory.o rle.o value.o

# link the object files together

main: $(OBJ_FILES)
	$(CC) $(C_FLAGS2) $^ -o main

# compile each src file to an object

$(OBJ_FILES): %.o: %.c
	$(CC) $(C_FLAGS1) $^ -o $@

clean:
	rm -f ./main && rm -f ./*.o
