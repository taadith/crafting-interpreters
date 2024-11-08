#include <stdio.h>
#include <string.h>

#include "common.h"
#include "scanner.h"

typedef struct {
    const char* start;
    const char* current;
    int line;
} Scanner;

Scanner scanner;

void initScanner(const char* src) {
    // start at the very first char...
    scanner.start = src;
    scanner.current = src;

    // ... on the very first line
    scanner.line = 1;
}