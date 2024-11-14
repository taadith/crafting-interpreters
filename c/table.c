#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"

// constructor for hash table
void initTable(Table* table) {
    table -> count = 0;
    table -> capacity = 0;
    table -> entries = 0;
}

// frees the hash table
void freeTable(Table* table) {
    FREE_ARRAY(Entry, table -> entries, table -> capacity);
    initTable(table);
}