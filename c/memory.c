#include <stdlib.h>

#include "memory.h"
#include "vm.h"

void* reallocate(void* ptr, size_t oldSize, size_t newSize) {
    if (newSize == 0) {
        free(ptr);
        return NULL;
    }

    void* res = realloc(ptr, newSize);
    if (res == NULL)
        exit(1);

    return res;
}

void* new_realloc(void* ptr, size_t newSize) {
    if (newSize == 0) {
        free(ptr);
        return NULL;
    }

    void* res = realloc(ptr, newSize);
    if (res == NULL)
        exit(1);

    return res;
}

static void freeObject(Obj* object) {
    switch (object -> type) {
        case OBJ_STRING: {
            ObjString* str = (ObjString*)object;

            // free the char arr and then free the ObjString
            FREE_ARRAY(char, str -> chars, str -> length + 1);
            FREE(ObjString, object);

            break;
        }
    }
}

void freeObjects(void) {
    // remember, this is the head!
    Obj* object = vm.objects;

    while(object != NULL) {
        Obj* next = object -> next;
        freeObject(object);
        object = next;
    }
}
