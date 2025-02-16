#include <stdio.h>

#include "common.h"
#include "rle.h"

void test_rle_easy1(void) {
    RunLengthEncoding rle;
    initRunLengthEncoding(&rle);

    writeRunLengthEncoding(&rle, 1);
    writeRunLengthEncoding(&rle, 1);
    
    printRunLengthEncoding(&rle, "test_rle_easy1");
    freeRunLengthEncoding(&rle);
}

void test_rle_easy2(void) {
    RunLengthEncoding rle;
    initRunLengthEncoding(&rle);

    writeRunLengthEncoding(&rle, 1);
    writeRunLengthEncoding(&rle, 1);

    writeRunLengthEncoding(&rle, 2);
    writeRunLengthEncoding(&rle, 2);
    
    printRunLengthEncoding(&rle, "test_rle_easy2");
    freeRunLengthEncoding(&rle);
}

void test_rle_easy3(void) {
    RunLengthEncoding rle;
    initRunLengthEncoding(&rle);

    writeRunLengthEncoding(&rle, 1);
    writeRunLengthEncoding(&rle, 2);
    writeRunLengthEncoding(&rle, 3);
    writeRunLengthEncoding(&rle, 4);
    writeRunLengthEncoding(&rle, 5);
    writeRunLengthEncoding(&rle, 6);
    writeRunLengthEncoding(&rle, 7);
    writeRunLengthEncoding(&rle, 8);
    writeRunLengthEncoding(&rle, 9);
    
    printRunLengthEncoding(&rle, "test_rle_easy3");
    freeRunLengthEncoding(&rle);
}

void test_rle_medium1(void) {
    RunLengthEncoding rle;
    initRunLengthEncoding(&rle);

    writeRunLengthEncoding(&rle, 1);
    writeRunLengthEncoding(&rle, 1);
    writeRunLengthEncoding(&rle, 2);
    writeRunLengthEncoding(&rle, 2);
    writeRunLengthEncoding(&rle, 3);
    writeRunLengthEncoding(&rle, 3);
    writeRunLengthEncoding(&rle, 4);
    writeRunLengthEncoding(&rle, 4);
    writeRunLengthEncoding(&rle, 5);
    writeRunLengthEncoding(&rle, 5);
    writeRunLengthEncoding(&rle, 6);
    writeRunLengthEncoding(&rle, 6);
    writeRunLengthEncoding(&rle, 7);
    writeRunLengthEncoding(&rle, 7);
    writeRunLengthEncoding(&rle, 8);
    writeRunLengthEncoding(&rle, 8);
    writeRunLengthEncoding(&rle, 9);
    writeRunLengthEncoding(&rle, 9);
    
    printRunLengthEncoding(&rle, "test_rle_medium1");
    freeRunLengthEncoding(&rle);
}

void test_getValueAtIndex_easy1(void) {
    RunLengthEncoding rle;
    initRunLengthEncoding(&rle);

    writeRunLengthEncoding(&rle, 1);
    writeRunLengthEncoding(&rle, 1);

    for(int i = 0; i < 2; i++) {
        printf("The value at index %d is %d\n", i, getValueAtIndex(&rle, i));
    }
    freeRunLengthEncoding(&rle);
}

void test_getValueAtIndex_easy2(void) {
    RunLengthEncoding rle;
    initRunLengthEncoding(&rle);

    writeRunLengthEncoding(&rle, 1);
    writeRunLengthEncoding(&rle, 1);

    writeRunLengthEncoding(&rle, 2);
    writeRunLengthEncoding(&rle, 2);

    for(int i = 0; i < 4; i++) {
        printf("The value at index %d is %d\n", i, getValueAtIndex(&rle, i));
    }
    freeRunLengthEncoding(&rle);
}

void test_getValueAtIndex_easy3(void) {
    RunLengthEncoding rle;
    initRunLengthEncoding(&rle);

    writeRunLengthEncoding(&rle, 1);
    writeRunLengthEncoding(&rle, 2);
    writeRunLengthEncoding(&rle, 3);
    writeRunLengthEncoding(&rle, 4);
    writeRunLengthEncoding(&rle, 5);
    writeRunLengthEncoding(&rle, 6);
    writeRunLengthEncoding(&rle, 7);
    writeRunLengthEncoding(&rle, 8);
    writeRunLengthEncoding(&rle, 9);
    
    for(int i = 0; i < 9; i++) {
        printf("The value at index %d is %d\n", i, getValueAtIndex(&rle, i));
    }
    freeRunLengthEncoding(&rle);
}

void test_getValueAtIndex_medium1(void) {
    RunLengthEncoding rle;
    initRunLengthEncoding(&rle);

    writeRunLengthEncoding(&rle, 1);
    writeRunLengthEncoding(&rle, 1);
    writeRunLengthEncoding(&rle, 2);
    writeRunLengthEncoding(&rle, 2);
    writeRunLengthEncoding(&rle, 3);
    writeRunLengthEncoding(&rle, 3);
    writeRunLengthEncoding(&rle, 4);
    writeRunLengthEncoding(&rle, 4);
    writeRunLengthEncoding(&rle, 5);
    writeRunLengthEncoding(&rle, 5);
    writeRunLengthEncoding(&rle, 6);
    writeRunLengthEncoding(&rle, 6);
    writeRunLengthEncoding(&rle, 7);
    writeRunLengthEncoding(&rle, 7);
    writeRunLengthEncoding(&rle, 8);
    writeRunLengthEncoding(&rle, 8);
    writeRunLengthEncoding(&rle, 9);
    writeRunLengthEncoding(&rle, 9);
    
    for(int i = 0; i < 18; i++) {
        printf("The value at index %d is %d\n", i, getValueAtIndex(&rle, i));
    }
    freeRunLengthEncoding(&rle);
}

int main(void) {
    // // basic tests
    // test_rle_easy1();
    // printf("\n");
    // test_rle_easy2();
    // printf("\n");
    // test_rle_easy3();
    // printf("\n");

    // test_rle_medium1();

    // getIndex tests
    printf("test_getValueAtIndex_easy1:\n");
    test_getValueAtIndex_easy1();
    printf("\n");

    printf("test_getValueAtIndex_easy2:\n");
    test_getValueAtIndex_easy2();
    printf("\n");
    
    printf("test_getValueAtIndex_easy3:\n");
    test_getValueAtIndex_easy3();
    printf("\n");
    
    printf("test_getValueAtIndex_medium1:\n");
    test_getValueAtIndex_medium1();
    printf("\n");

    return 0;
}
