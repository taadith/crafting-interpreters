#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"
#include "vm.h"

// helps avoid the need to redundantly cast...
// ... a void* back to the desired type
#define ALLOCATE_OBJ(type, objectType) \
    (type*)allocateObject(sizeof(type), objectType)

// object constructor
static Obj* allocateObject(size_t size, ObjType type) {
    // creates a new Object on the heap
    Obj* object = (Obj*)reallocate(NULL, 0, size);

    // initializes the type
    object -> type = type;

    // insert object as the new...
    // ... head of the (singly) linked list
    object -> next = vm.objects;
    vm.objects = object;

    // returns the ptr
    return object;
}

// string constructor
static ObjString* allocateString(char* chars, int length,
                                 uint32_t hash) {
    // creates a new ObjString on the heap
    ObjString* string = ALLOCATE_OBJ(ObjString, OBJ_STRING);

    // initalizes the fields
    string -> length = length;
    string -> chars = chars;
    string -> hash = hash;

    // add string to the "interned" strings hash table
    tableSet(&vm.strings, string, NIL_VAL);

    // returns the ptr
    return string;
}

ObjFunction* newFunction() {
    // allocate memory and initialize the object's header...
    // ... so the VM knows what type of object it is
    ObjFunction* function = ALLOCATE_OBJ(ObjFunction, OBJ_FUNCTION);
    
    // set the function up in a blank state
    function -> arity = 0;
    function -> name = NULL;
    initChunk(&function -> chunk);

    return function;

// utilizes the FNV-1a algorithm
static uint32_t hashString(const char* key, int length) {
    // start w/ an initial hash value
    uint32_t hash = 2166136261u;

    // walk the data to be hashed
    for(int i = 0; i < length; i++) {
        // for each byte, mix the...
        // ... bits into the hash value
        hash ^= (uint8_t)key[i];

        // scramble resulting bits around
        hash *= 16777619;
    }

    return hash;
}

// claims ownership of the c string given
ObjString* takeString(char* chars, int length) {
    // hash the string
    uint32_t hash = hashString(chars, length);

    // look up the string in the string table first...
    ObjString* interned = tableFindString(&vm.strings, chars, length, hash);
    // ... if the string is found, return a ref to it
    if (interned != NULL) {
        // free the memory for the string that was passed in...
        // ... don't need the duplicate string
        FREE_ARRAY(char, chars, length + 1);
        return interned;
    }

    return allocateString(chars, length, hash);
}

// conservatively creates a copy of the chars on the heap
ObjString* copyString(const char* chars, int length) {
    // hash the copied string
    uint32_t hash = hashString(chars, length);

    // look up the string in the string table first...
    ObjString* interned = tableFindString(&vm.strings, chars, length, hash);
    // ... if the string is found, return a ref to it
    if (interned!= NULL)
        return interned;

    // allocate a new array on the heap big enough...
    // ... for the string's chars and the trailing terminator
    char* heapChars = ALLOCATE(char, length + 1);

    // copy over data...
    memcpy(heapChars, chars, length);
    // ... and terminate it
    heapChars[length] = '\0';

    // string constructor
    return allocateString(heapChars, length, hash);
}

// a function knows it name, may as well print it!
static void printFunction(ObjFunction* function) {
    print("<fn %s>", function -> name -> chars);
}

void printObject(Value value) {
    switch (OBJ_TYPE(value)) {
        case OBJ_FUNCTION:
            printFunction(AS_FUNCTION(value));
            break;
        case OBJ_STRING:
            printf("%s", AS_CSTRING(value));
    }
}
