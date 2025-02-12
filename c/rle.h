#ifndef clox_rle_h
#define clox_rle_h

#include "common.h"
#include "memory.h"

typedef struct {
    int* values;
    int* lengths;
    int count;
    int capacity;
} RunLengthEncoding;

// initializes a RLE
void initRunLengthEncoding(RunLengthEncoding* rle);

// writes an integer to the RLE
void writeRunLengthEncoding(RunLengthEncoding* rle, int value);

// frees the RLE
void freeRunLengthEncoding(RunLengthEncoding* rle);

#endif
