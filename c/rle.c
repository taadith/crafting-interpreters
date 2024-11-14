#include <stdlib.h>
#include <stdio.h>

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
    // RLE cannot add new data
    if (rle -> capacity < rle -> count + 1) {
        int oldCapacity = rle -> capacity;

        // double capacity (or set to 8)
        rle -> capacity = GROW_CAPACITY(oldCapacity);

        // double size of arrays (or set to 8)
        rle -> multiple = GROW_ARRAY(int, rle -> multiple,
                                   oldCapacity, rle -> capacity);
        rle -> data = GROW_ARRAY(int, rle -> data,
                                    oldCapacity, rle -> capacity);
    }

    rle -> multiple[rle -> count] = 1;
    rle -> data[rle -> count] = newData;
    rle -> totalData++;

    if (rle -> count > 0 && rle -> data[rle -> count] == rle -> data[(rle -> count) - 1])
        rle -> multiple[(rle -> count) - 1]++;
    else
        rle -> count++;
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
    for(int i = 0; i < rle -> count; i++)
        printf("%d x %d\n", rle -> multiple[i], rle -> data[i]);
}
