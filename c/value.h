#ifndef clox_value_h
#define clox_value_h

#include "common.h"

typedef double Value;

typedef struct {
    int capacity;
    int count;
    Value* values;
} ValueArray;

// initialize a ValueArray
void initValueArray(ValueArray* array);

// write Value to ValueArray
void writeValueArray(ValueArray* array, Value value);

// free the ValueArray
void freeValueArray(ValueArray* array);

// prints Value
void printValue(Value value);

#endif
