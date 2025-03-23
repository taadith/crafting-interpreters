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
    switch(value.type) {
        case VAL_BOOL:
            printf(AS_BOOL(value) ? "true" : "false");
            break;
        case VAL_NIL:
            printf("nil");
            break;
        case VAL_NUMBER:
            printf("%g", AS_NUMBER(value));
            break;
    }
}

bool valuesEqual(Value a, Value b) {
    // check if types are even same
    if (a.type != b.type)
        return false;

    // value comparison
    switch (a.type) {
        case VAL_BOOL:
            return AS_BOOL(a) == AS_BOOL(b);
        case VAL_NIL:
            return true;
        case VAL_NUMBER:
            return AS_NUMBER(a) == AS_NUMBER(b);

        // in theory, unreachable
        default:
            return false;
    }
}
