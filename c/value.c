#include <stdio.h>

#include "memory.h"
#include "value.h"

// initialize a ValueArray
void initValueArray(ValueArray* array) {
    array -> values = NULL;
    array -> capacity = 0;
    array -> count = 0;
}

// write Value to ValueArray
void writeValueArray(ValueArray* array, Value value) {
    if (array -> capacity < array -> count + 1) {
        int oldCapacity = array -> capacity;
        array -> capacity = GROW_CAPACITY(oldCapacity);
        array -> values = GROW_ARRAY(Value, array -> values,
                                     oldCapacity, array -> capacity);

        // array -> values = NEW_GROW_ARRAY(Value, array -> values,
        //    array -> capacity);
    }

    array -> values[array -> count] = value;
    array -> count++;
}

// free the ValueArray
void freeValueArray(ValueArray* array) {
    FREE_ARRAY(Value, array -> values, array -> capacity);
    // NEW_FREE_ARRAY(array -> values);

    initValueArray(array);
}

// prints Value
void printValue(Value value) {
    printf("%g", value);
}
