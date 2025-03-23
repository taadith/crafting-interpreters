CC := clang
C_FLAGS1 := -Wall -Wextra -Wpedantic -g -c
C_FLAGS2 := -g

MAIN_OBJ_FILES := chunk.o compiler.o debug.o main.o memory.o object.o rle.o scanner.o value.o vm.o
CHUNK_TEST_OBJ_FILES := chunk.o chunk_test.o debug.o memory.o rle.o value.o
RLE_TEST_OBJ_FILES := memory.o rle.o rle_test.o

# link the object files together

main: $(MAIN_OBJ_FILES)
	$(CC) $(C_FLAGS2) $^ -o main

chunk_test: $(CHUNK_TEST_OBJ_FILES)
	$(CC) $(C_FLAGS2) $^ -o chunk_test

rle_test: $(RLE_TEST_OBJ_FILES)
	$(CC) $(C_FLAGS2) $^ -o rle_test 

# compile each src file to an object

$(MAIN_OBJ_FILES): %.o: %.c
	$(CC) $(C_FLAGS1) $^ -o $@

$(CHUNK_TEST_OBJ_FILES): %.o: %.c
	$(CC) $(C_FLAGS1) $^ -o $@

$(RLE_TEST_OBJ_FILES): %.o: %.c
	$(CC) $(C_FLAGS1) $^ -o $@

# helper commands

clean:
	rm -f ./chunk_test ./main ./rle_test ./*.o
