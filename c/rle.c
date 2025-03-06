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
        // rle -> values = GROW_ARRAY(int, rle -> values,
        //     0, rle -> capacity);
        // rle -> lengths = GROW_ARRAY(int, rle -> lengths,
        //     0, rle -> capacity);

        rle -> values = NEW_GROW_ARRAY(int, rle -> values,
            rle -> capacity);
        rle -> lengths = NEW_GROW_ARRAY(int, rle -> lengths,
            rle -> capacity);

        rle -> values[0] = value;
        rle -> lengths[0] = 1;
        rle -> count++;
    }
    
    // adding a pre-existing value
    else if (rle -> values[rle -> count - 1] == value) {
        rle -> lengths[rle -> count - 1]++;
    }

    // adding a new value
    else {
        if (rle -> capacity < rle -> count + 1) {
            int oldCapacity = rle -> capacity;
            rle -> capacity = GROW_CAPACITY(oldCapacity);
            // rle -> values = GROW_ARRAY(int, rle -> values,
            //                            oldCapacity, rle -> capacity);
            // rle -> lengths = GROW_ARRAY(int, rle -> lengths,
            //                            oldCapacity, rle -> capacity);

            rle -> values = NEW_GROW_ARRAY(int, rle -> values,
               rle -> capacity);
            rle -> lengths = NEW_GROW_ARRAY(int, rle -> lengths,
               rle -> capacity);

        }
        rle -> values[rle -> count] = value;
        rle -> lengths[rle -> count] = 1;
        rle -> count++;
    }
}

// frees the RLE
void freeRunLengthEncoding(RunLengthEncoding* rle) {
    // FREE_ARRAY(int, rle -> values, rle -> capacity);
    // FREE_ARRAY(int, rle -> lengths, rle -> capacity);

    NEW_FREE_ARRAY(rle -> values);
    NEW_FREE_ARRAY(rle -> lengths);

    initRunLengthEncoding(rle);
}

// grabs value at index
int getValueAtIndex(RunLengthEncoding* rle, int index) {
    int total_count = 0;
    int rle_index = 0;
    do {
        total_count += rle -> lengths[rle_index];
        rle_index++;
    } while (total_count - 1 < index);
    return rle -> values[rle_index - 1];
}

// prints the RLE
void printRunLengthEncoding(RunLengthEncoding* rle, const char* name) {
    printf("%s:\n", name);
    printf("Values --> \n[");
    for(int i = 0; i < rle -> count; i++) {
        printf("%d", rle -> values[i]);
        if (i < rle -> count - 1) {
            printf(", ");
        }
    }
    printf("]\n");

    printf("Lengths --> \n[");
    for(int i = 0; i < rle -> count; i++) {
        printf("%d", rle -> lengths[i]);
        if (i < rle -> count - 1) {
            printf(", ");
        }
    }
    printf("]\n");
}
