#ifndef clox_rle_h
#define clox_rle_h

#include "common.h"

typedef struct {
    int count;
    int capacity;

    int* multiple;
    int* data;
    size_t totalData;
} RunLengthEncoding;

// creates a RLE
void initRunLengthEncoding(RunLengthEncoding* rle);

// writes new data to the RLE
void writeRunLengthEncoding(RunLengthEncoding* rle, int newData);

// frees the RLE
void freeRunLengthEncoding(RunLengthEncoding* rle);

// prints out the RLE
void printRunLengthEncoding(RunLengthEncoding* rle);

#endif
