#include <stdlib.h>

#include "rle.h"
#include "memory.h"

// creates a RLE
void initRunLengthEncoding(RunLengthEncoding* rle) {
    rle -> count = 0;
    rle -> capacity = 0;

    rle -> multiple = NULL;
    rle -> data = NULL;
    rle -> totalData = 0;
}

// writes new data to the RLE
void writeRunLengthEncoding(RunLengthEncoding* rle, int newData) {

}

// frees the RLE
void freeRunLengthEncoding(RunLengthEncoding* rle) {
    FREE_ARRAY(int, rle -> multiple, rle -> capacity);
    FREE_ARRAY(int, rle -> data, rle -> capacity);

    // zeroing out the values
    initRunLengthEncoding(rle);
}

// prints out the RLE
void printRunLengthEncoding(RunLengthEncoding* rle) {

}