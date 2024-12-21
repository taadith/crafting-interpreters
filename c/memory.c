#include <stdlib.h>

#include "memory.h"
#include "vm.h"

void* reallocate(void* ptr, size_t oldSize, size_t newSize) {
    // oldSize == 0 && newSize > 0
    if (newSize == 0) {
        free(ptr);
        return NULL;
    }

    // handles all other cases!
    // tries to change the size of the allocation...
    // ... pointed to by ptr and returns ptr
    void* result = realloc(ptr, newSize);

    // failed allocation
    if (result == NULL)
        exit(1);
    
    return result;
}

static void freeObject(Obj* object) {
    switch(object -> type) {
        case OBJ_FUNCTION: {
            ObjFunction* function = (ObjFunction*) object;

            // functions own their chunk, so we have to call...
            // the Chunk's destructor-like func
            freeChunk(&function -> chunk);

            FREE(ObjFunction, object);
            break;
        }
        case OBJ_STRING: {
            ObjString* string = (ObjString*) object;
            
            // free the char array...
            FREE_ARRAY(char, string -> chars, string -> length + 1);
            
            // ... and then we free the ObjString
            FREE(ObjString, object);
        }
    }
}

void freeObjects(void) {
    Obj* object = vm.objects;

    // walking a linked list...
    // ... and freeing its nodes
    while(object != NULL) {
        Obj* next = object -> next;
        freeObject(object);
        object = next;
    }
}
