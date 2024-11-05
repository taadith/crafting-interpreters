#ifndef clox_rle_h
#define clox_rle_h

typedef struct {
    int count;
    int capacity;

    int* multiple;
    int* data;
    int totalData;
} RunLengthEncoding;

#endif