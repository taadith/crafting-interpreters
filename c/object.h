#ifndef clox_object_h
#define clox_object_h

#include "common.h"
#include "value.h"

// extracts object type tag from a given Value
#define OBJ_TYPE(value)     (AS_OBJ(value) -> type)

// is the Obj a OBJ_STRING?
#define IS_STRING(value)    isObjType(value, OBJ_STRING)

// returns the ObjString* ptr
#define AS_STRING(value)    ((ObjString*) AS_OBJ(value))

// steps through the ObjString* ptr...
// ... to return the char array itself
#define AS_CSTRING(value)   (((ObjString*) AS_OBJ(value)) -> chars)

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

static inline bool isObjType(Value value, ObjType type) {
    return IS_OBJ(value) && (AS_OBJ(value) -> type == type);
}

#endif