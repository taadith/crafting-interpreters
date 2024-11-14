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

// take a key and figure out which bucket in the array it should go in...
// ... returns a ptr to that bucket -- the address of the Entry in the array
static Entry* findEntry(Entry* entries, int capacity,
                        ObjString* key) {
    uint32_t index = (key -> hash) % capacity;

    // first time we pass a tombstone, store it here
    Entry* tombstone = NULL;

    for(;;) {
        Entry* entry = &entries[index];

        // empty entry OR a tombstone
        if (entry -> key == NULL) {
            // empty entry, so if we have passed a tombstone...
            // ... return its bucket instead of the later empty one
            if (IS_NIL(entry -> value))
                return tombstone != NULL ? tombstone : entry;
            
            // found a tombstone, so have this be the entry
            else {
                if (tombstone == NULL)
                    tombstone == entry;
            }
        }
        // found the key!
        else if (entry -> key == key)   // returns true if they're the exact same string in memory...
            return entry;
        
        // keep looking!
        index = (index + 1) % capacity;
    }
}

// given a key, look up the corresponding value
bool tableGet(Table* table, ObjString* key, Value* value) {
    // empty table, so key doesn't exist
    if (table -> count == 0)
        return false;
    
    // look up the entry w/ the corresponding key
    Entry* entry = findEntry(table -> entries, table -> capacity, key);
    
    // empty bucket, so no Entry w/ key
    if (entry -> key == NULL)
        return false;
    
    // found a match
    *value = entry -> value;
    return true;
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

    // recalculate bc we don't copy tombstones over
    table -> count = 0;

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

        // increment each time a non-tombstone entry is found
        table -> count++;
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
    
    // increment count if it's an empty bucket...
    // ... not counting tombstones
    bool isNewKey = entry -> key == NULL;
    if (isNewKey && IS_NIL(entry -> value))
        table -> count++;

    entry -> key = key;
    entry -> value = value;

    return isNewKey;
}

// deleting an entry from the given hash table
bool tableDelete(Table* table, ObjString* key) {
    // empty table, so no key exists to delete
    if (table -> count == 0)
        return false;

    // find the entry to delete
    Entry* entry = findEntry(table -> entries, table -> capacity, key);

    // empty bucket, so no Entry w/ key to begin with
    if (entry -> key == NULL)
        return false;

    // place a tombstone in the entry
    entry -> key = NULL;
    entry -> value = BOOL_VAL(true);

    return true;
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

// what does this do??
ObjString* tableFindString(Table* table, const char* chars,
                           int length, uint32_t hash) {
    if (table -> count = 0)
        return NULL;
    
    uint32_t index = hash % (table -> capacity);
    for(;;) {
        Entry* entry = &(table -> entries[index]);

        // stop if we find an empty non-tombstone entry
        if (entry -> key == NULL) {
            if (IS_NIL(entry -> value))
                return NULL;
        }

        // found it!
        else if (entry -> key -> length == length &&
                 entry -> key -> hash == hash &&
                 memcmp(entry -> key -> chars, chars, length) == 0)
            return entry -> key;
        
        // keep looking!
        index = (index + 1) % (table -> capacity);
    }
}