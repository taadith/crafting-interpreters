#ifndef clox_table_h
#define clox_table_h

#include "common.h"
#include "value.h"

// the key/value pair
typedef struct {
    ObjString* key;
    Value value;
} Entry;

// the hash table
typedef struct {
    // load factor = count / capacity
    int count;      // # of currently stored key/value pairs
    int capacity;   // allocated size of the array

    // it's just an array of entries
    Entry* entries;
} Table;

// constructor for hash table
void initTable(Table* table);

// frees the hash table
void freeTable(Table* table);

// given a key, look up the corresponding value
bool tableGet(Table* table, ObjString* key, Value* value);

// adds given key/value pair to the given hash table
bool tableSet(Table* table, ObjString* key, Value value);

// deleting an entry from the given hash table
bool tableDelete(Table* table, ObjString* key);

// copies entries of one hash table to another
bool tableAddAll(Table* from, Table* to);

#endif
