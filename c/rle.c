#include <stdio.h>

#include "rle.h"
#include "memory.h"

// initializes a RLE
void initRunLengthEncoding(RunLengthEncoding* rle) {
    rle -> values = NULL;
    rle -> lengths = NULL;
    rle -> count = 0;
    rle -> capacity = 0;
}

// writes an integer to the RLE
void writeRunLengthEncoding(RunLengthEncoding* rle, int value) {
    // new RLE!
    if (rle -> capacity == 0) {
        rle -> capacity = GROW_CAPACITY(0);
        rle -> values = GROW_ARRAY(int, rle -> values,
                                     0, rle -> capacity);
        rle -> lengths = GROW_ARRAY(int, rle -> lengths,
                                     0, rle -> capacity);

        rle -> values[0] = value;
        rle -> lengths[0] = 1;
        rle -> count++;
    }
    
    // adding a pre-existing value
    else if (rle -> values[rle -> count] == value) {
        rle -> lengths[rle -> count]++;
    }

    // adding a new value
    else {
        int oldCapacity = rle -> capacity;
        rle -> capacity = GROW_CAPACITY(oldCapacity);
        rle -> values = GROW_ARRAY(int, rle -> values,
                                   oldCapacity, rle -> capacity);
        rle -> lengths = GROW_ARRAY(int, rle -> lengths,
                                   oldCapacity, rle -> capacity);

        rle -> values[rle -> count] = value;
        rle -> count++;

    }
}

// frees the RLE
void freeRunLengthEncoding(RunLengthEncoding* rle) {
    FREE_ARRAY(int, rle -> values, rle -> capacity);
    FREE_ARRAY(int, rle -> lengths, rle -> capacity);
    initRunLengthEncoding(rle);
}
