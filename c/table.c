#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"

// picked somewhat arbitrarily
#define TABLE_MAX_LOAD 0.75

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

// findEntry()'s job is to take a key and figure out which bucket in the array it should go in
// returns a ptr to that bucket -- the address of the Entry in the array
static Entry* findEntry(Entry* entries, int capacity,
                        ObjString* key) {
    uint32_t index = (key -> hash) % capacity;

    for(;;) {
        Entry* entry = &entries[index];

        // found that the Entry's key matches w/ our key...
        // ... or found an empty Entry
        if (entry -> key == key || entry -> key == NULL)
            return entry;
        
        // keep looking!
        index = (index + 1) % capacity;
    }
}

bool tableSet(Table* table, ObjString* key, Value value) {
    // checking if arrays exists OR if the array is big enough
    if (table -> count + 1 > table -> capacity * TABLE_MAX_LOAD) {
        int newCapacity = GROW_CAPACITY(table -> capacity);
        adjustCapacity(table, newCapacity);
    }
    
    // find entry
    Entry* entry = findEntry(table -> entries, table -> capacity, key);
    
    // increment count if it's an untouched entry
    bool isNewKey = entry -> key == NULL;
    if (isNewKey)
        table -> count++;

    entry -> key = key;
    entry -> value = value;

    return isNewKey;
}
