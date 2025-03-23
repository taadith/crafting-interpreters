#ifndef clox_object_h
#define clox_object_h

#include "common.h"
#include "value.h"

// extracts the object type tag...
// ... from a given Value
#define OBJ_TYPE(value)         (AS_OBJ(value) -> type)

// object type checking
#define IS_STRING(value)        isObjType(value, OBJ_STRING)

// converting object or C value to it's respective C value or object
#define AS_STRING(value)        ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value)       (((ObjString*)AS_OBJ(value)) -> chars)

typedef enum {
    OBJ_STRING,
} ObjType;

struct Obj {
    ObjType type;
};

struct ObjString {
    Obj obj;
    int length;
    char* chars;
};

ObjString* copyString(const char* chars, int length);

// helper function for checking object type
static inline bool isObjType(Value value, ObjType type) {
    return IS_OBJ(value) && AS_OBJ(value) -> type == type;
}

#endif
