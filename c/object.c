#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "value.h"
#include "vm.h"

#define ALLOCATE_OBJ(type, objectType) \
    (type*)allocateObject(sizeof(type), objectType)

// akin to an Obj constructor
static Obj* allocateObject(size_t size, ObjType type) {
    // new Obj is created in the heap
    Obj* object = (Obj*)reallocate(NULL, 0, size);

    // field is initialized
    object -> type = type;

    // insert new Obj at the head...
    // ... or the tail depending on where you're looking
    object -> next = vm.objects;
    vm.objects = object;

    return object;
}

// akin to an ObjString constructor
static ObjString* allocateString(char* chars, int length) {
    // new ObjString is created in the heap
    ObjString* string = ALLOCATE_OBJ(ObjString, OBJ_STRING);

    // fields are initialized
    string -> length = length;
    string -> chars = chars;

    return string;
}

ObjString* takeString(char* chars, int length) {
    return allocateString(chars, length);
}

ObjString* copyString(const char* chars, int length) {
    // allocate a new array on the heap big enough...
    // ... for the string's chars and a trailing...
    // ... terminator
    char* heapChars = ALLOCATE(char, length + 1);

    // copy over characters from the lexeme...
    // ... and terminates it...
    // ... the lexeme pts at a range of chars...
    // ... inside the monolithic src string...
    // ... and isn't terminated itself
    memcpy(heapChars, chars, length);
    heapChars[length] = '\0';

    // construct the string
    return allocateString(heapChars, length);
}

void printObject(Value value) {
    switch (OBJ_TYPE(value)) {
        case OBJ_STRING:
            printf("%s", AS_CSTRING(value));
            break;
    }
}
