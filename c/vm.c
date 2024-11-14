#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "object.h"
#include "memory.h"
#include "vm.h"

VM vm;

void push(Value value) {
    // set stackTop location to new value
    *(vm.stackTop) = value;

    // move the stackTop just...
    // ... past the last item
    vm.stackTop++;
}

Value pop(void) {
    // move stack top down
    vm.stackTop--;

    // return item where stack top moved too...
    // ... bc its just past the last item
    return *(vm.stackTop);
}

// returns a Value <distance> "Values" down...
// ... from the top of the stack
static Value peek(int distance) {
    return vm.stackTop[-1 - distance];
}

static void resetStack(void) {
    // set stackTop to stack[0]
    vm.stackTop = vm.stack;
}

static bool isFalsey(Value value) {
    return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static void concatenate(void) {
    ObjString* b = AS_STRING(pop());
    ObjString* a = AS_STRING(pop());

    // new combined length
    int length = a -> length + b -> length;

    // allocate space for C string
    char* chars = ALLOCATE(char, length + 1);

    // copy a -> chars and then b -> chars
    memcpy(chars, a -> chars, a -> length);
    memcpy(chars + a -> length, b -> chars, b -> length);

    // set null-terminated char at end
    chars[length] = '\0';

    ObjString* result = takeString(chars, length);
    push(OBJ_VAL(result));
}

static void runtimeError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    size_t instructionIndex = vm.ip - (vm.chunk -> code) - 1;
    int line = getLine(vm.chunk, instructionIndex);
    fprintf(stderr, "[line %d] in script\n", line);
    resetStack();
}

void initVM(void) {
    resetStack();
    vm.objects = NULL;

    // initialize the "interned" strings hash table
    initTable(&vm.strings);
}

void freeVM(void) {
    // clean up the "interned" strings hash table
    freeTable(&vm.strings);

    // clean up the objects on the heap
    freeObjects();
}

static InterpretResult run(void) {
// reads current byte pointed at by ip...
// ... and then advances the ip
#define READ_BYTE() (*vm.ip++)

// reads the next byte from the bytecode...
// ... treats the resulting # as an index...
// ... and looks up the corresponding Value in the chunk's constant table
#define READ_CONSTANT() (vm.chunk -> constants.values[READ_BYTE()])

// pop values off the stack...
// ... and then push the result
#define BINARY_OP(valueType, op) \
    do { \
        if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
            runtimeError("operands must be numbers"); \
            return INTERPRET_RUNTIME_ERROR; \
        } \
        double b = AS_NUMBER(pop()); \
        double a = AS_NUMBER(pop()); \
        push(valueType(a op b)); \
    } while (false)

    for(;;) {

// dynamic debugging if flag is defined
#ifdef DEBUG_TRACE_EXECUTION
        printf("\t\t");
        
        // print each value in the array
        for (Value* slot = vm.stack; slot < vm.stackTop; slot++) {
            printf("[ ");
            printValue(*slot);
            printf(" ]");
        }
        printf("\n");

        disassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk -> code));
#endif

        // first byte of any instruction is the opcode
        uint8_t instruction;
        switch(instruction = READ_BYTE()) {
            case OP_CONSTANT: {
                // constant gets pushed to the stack
                Value constant = READ_CONSTANT();
                push(constant);

                break;
            }

            case OP_NIL:
                push(NIL_VAL);
                break;
            
            case OP_TRUE:
                push(BOOL_VAL(true));
                break;
            
            case OP_FALSE:
                push(BOOL_VAL(false));
                break;
            
            case OP_EQUAL: {
                Value b = pop();
                Value a = pop();
                push(BOOL_VAL(valuesEqual(a, b)));
                break;
            }

            case OP_GREATER:
                BINARY_OP(BOOL_VAL, >);
                break;
            
            case OP_LESS:
                BINARY_OP(BOOL_VAL, <);
                break;

            case OP_ADD: {
                if (IS_STRING(peek(0)) && IS_STRING(peek(1)))
                    concatenate();
                else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) {
                    double b = AS_NUMBER(pop());
                    double a = AS_NUMBER(pop());
                    push(NUMBER_VAL(a + b));
                }
                else {
                    runtimeError("operands must be two numbers or two strings");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }

            case OP_SUBTRACT: {
                BINARY_OP(NUMBER_VAL, -);
                break;
            }

            case OP_MULTIPLY: {
                BINARY_OP(NUMBER_VAL, *);
                break;
            }

            case OP_DIVIDE: {
                BINARY_OP(NUMBER_VAL, /);
                break;
            }

            case OP_NOT:
                // pops one operand, performs logical not, pushes result
                push(BOOL_VAL(isFalsey(pop())));
                break;

            case OP_NEGATE: {
                // check that Value on top of stack is a number
                if (!IS_NUMBER(peek(0))) {
                    runtimeError("operand must be a number");
                    return INTERPRET_RUNTIME_ERROR;
                }
                push(NUMBER_VAL(-AS_NUMBER(pop())));
                break;
            }

            // simply pop and print the value
            case OP_PRINT: {
                printValue(pop());
                printf("\n");
                break;
            }


            // exit the interpreter
            case OP_RETURN: {
                return INTERPRET_OK;
            }
        }
    }

#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}

InterpretResult interpret(const char* src) {
    // create a new empty chunk
    Chunk chunk;
    initChunk(&chunk);

    // pass it over to the compiler which takes the...
    // ... user's program and fills the chunk w/ bytecode...
    // ... unless there's an error
    if (!compile(src, &chunk)) {
        // discard the unusable chunk
        freeChunk(&chunk);
        return INTERPRET_COMPILE_ERROR;
    }

    // send the completed chunk over to the VM
    vm.chunk = &chunk;
    vm.ip = vm.chunk -> code;

    // VM executes code
    InterpretResult result = run();

    // the chunk is freed and now all done!
    freeChunk(&chunk);
    return result;
}
