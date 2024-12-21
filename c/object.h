#ifndef clox_object_h
#define clox_object_h

#include "common.h"
#include "chunk.h"
#include "value.h"

// extracts object type tag from a given Value
#define OBJ_TYPE(value)     (AS_OBJ(value) -> type)

// is the Obj a OBJ_FUNCTION?
#define IS_FUNCTION(value)  isObjType(value, OBJ_FUNCTION)

// is the Obj a OBJ_STRING?
#define IS_STRING(value)    isObjType(value, OBJ_STRING)

// returns the ObjFunction* ptr
#define AS_FUNCTION(value)  ((ObjFunction*) AS_OBJ(value))
// returns the ObjString* ptr
#define AS_STRING(value)    ((ObjString*) AS_OBJ(value))

// steps through the ObjString* ptr...
// ... to return the char array itself
#define AS_CSTRING(value)   (((ObjString*) AS_OBJ(value)) -> chars)

typedef enum {
    OBJ_FUNCTION,
    OBJ_STRING
} ObjType;

// struct itself is the linked list node for the GC
struct Obj {
    ObjType type;
    struct Obj* next;
};

// functions are first class in Lox...
// ... so they need to be Lox objects
typedef struct {
    Obj obj;
    int arity;  // stores # of params the function expects
    Chunk chunk;
    ObjString* name;
} ObjFunction;

// how strings are defined
struct ObjString {
    Obj obj;
    int length;
    char* chars;

    // caching the hash
    uint32_t hash;
};

ObjFunction* newFunction();

ObjString* takeString(char* chars, int length);

// allocate a new ObjString
ObjString* copyString(const char* chars, int length);

void printObject(Value value);

static inline bool isObjType(Value value, ObjType type) {
    return IS_OBJ(value) && (AS_OBJ(value) -> type == type);
}

#endif
