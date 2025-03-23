#ifndef clox_value_h
#define clox_value_h

#include "common.h"

typedef enum {
    VAL_BOOL,
    VAL_NIL,
    VAL_NUMBER
} ValueType;

typedef struct {
    // will take up four bytes
    ValueType type;

    // four-byte padding to keep...
    // ... double on the nearest...
    // ... eight-byte boundary
    
    // eight-byte union...
    // ... b/c of eight-byte double
    union {
        bool boolean;
        double number;
    } as;
} Value;

// returns `true` if the Value has that type
#define IS_BOOL(value)      ((value).type == VAL_BOOL)
#define IS_NIL(value)       ((value).type == VAL_NIL)
#define IS_NUMBER(value)    ((value).type == VAL_NUMBER)

// unwraps a Value of the right type and returns...
// ... the corresponding raw C value
#define AS_BOOL(value)      ((value).as.boolean)
#define AS_NUMBER(value)    ((value.as.number))

// each one of these takes a C value of the appropriate type...
// ... and produces a Value that has the correct type...
// ... tag and contains the underlying value
#define BOOL_VAL(value)     ((Value){VAL_BOOL, {.boolean = value}})
#define NIL_VAL             ((Value){VAL_NIL, {.number = 0}})
#define NUMBER_VAL(value)   ((Value){VAL_NUMBER, {.number = value}})

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
