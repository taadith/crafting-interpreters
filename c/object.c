#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"
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
static ObjString* allocateString(char* chars, int length) {
    // creates a new ObjString on the heap
    ObjString* string = ALLOCATE_OBJ(ObjString, OBJ_STRING);

    // initalizes the fields
    string -> length = length;
    string -> chars = chars;

    // returns the ptr
    return string;
}

// claims ownership of the c string given
ObjString* takeString(char* chars, int length) {
    return allocateString(chars, length);
}

// conservatively creates a copy of the chars on the heap
ObjString* copyString(const char* chars, int length) {
    // allocate a new array on the heap big enough...
    // ... for the string's chars and the trailing terminator
    char* heapChars = ALLOCATE(char, length + 1);

    // copy over data...
    memcpy(heapChars, chars, length);
    // ... and terminate it
    heapChars[length] = '\0';

    // string constructor
    return allocateString(heapChars, length);
}

void printObject(Value value) {
    switch (OBJ_TYPE(value)) {
        case OBJ_STRING:
            printf("%s", AS_CSTRING(value));
    }
}
