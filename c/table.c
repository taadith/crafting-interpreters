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

// changing table to a new capacity
static void adjustCapacity(Table* table, int capacity) {
    // create a bucket array w/ capacity entries
    Entry* entries = ALLOCATE(Entry, capacity);

    // initialize every element to be an empty bucket
    for(int i = 0; i < capacity; i++) {
        entries[i].key == NULL;
        entries[i].value = NIL_VAL;
    }

    // re-insert every entry into the new empty array
    for (int i = 0; i < table -> capacity; i++) {
        Entry* entry = &table -> entries[i];

        // empty entry, so skip!
        if (entry -> key == NULL)
            continue;

        // find a ptr to where the key should go
        Entry* dest = findEntry(entries, capacity, entry -> key);
        
        // initialize the key/value pair of the bucket
        dest -> key = entry -> key;
        dest -> value = entry -> value;
    }

    // release memory for old array
    FREE_ARRAY(Entry, table -> entries, table -> capacity);

    // replace the table's entries and...
    // ... capacity w/ the new versions
    table -> entries = entries;
    table -> capacity = capacity;
}

// adds given key/value pair to the given hash table
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

// copies entries of one hash table to another
void tableAddAll(Table* from, Table* to) {
    // walks the bucket array of the src hash table
    for(int i = 0; i < from -> capacity; i++) {
        Entry* entry = &(from -> entries[i]);

        // after finding a non-empty bucket...
        // ... add entry to dest hash table
        if (entry -> key != NULL)
            tableSet(to, entry -> key, entry -> value);
    }
}
