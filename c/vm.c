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

    initTable(&vm.globals);
    initTable(&vm.strings);
}

void freeVM(void) {
    freeTable(&vm.globals);
    freeTable(&vm.strings);

    // clean up the objects on the heap
    freeObjects();
}

// macro reads current byte pointed at by ip...
// ... and then advances the ip
#define READ_BYTE() (*vm.ip++)

// macro reads the next byte from the bytecode...
// ... treats the resulting # as an index...
// ... and looks up the corresponding Value in the chunk's constant table
#define READ_CONSTANT() (vm.chunk -> constants.values[READ_BYTE()])

// (1) increment the ip by 2 bytes:
// (2) vm.ip[-2] has the upper byte of the 16-byte value
//          -> 0b********--------
// (3) vm.ip[-1] has the lower byte of the 16-byte value:
//          -> 0b--------^^^^^^^^
// (4) these are | together:
//          -> 0b********^^^^^^^^
#define READ_SHORT() \
    (vm.ip += 2, (uint16_t)((vm.ip[-2] << 8) | vm.ip[-1]))

// macro reads a one-byte operand from the bytecode chunk...
// ... treats the operand as an index into the chunk's constant table...
#define READ_STRING() AS_STRING(READ_CONSTANT())

// macro pop values off the stack...
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

static InterpretResult run(void) {
    for(;;) {
// dynamic debugging if the flag is defined
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
            
            case OP_POP:
                pop();
                break;
            
            case OP_GET_LOCAL: {
                // takes a single-byte operand for the...
                // ... stack slot where the local lives
                uint8_t slot = READ_BYTE();

                // loads value from the slot index and...
                // ... then pushes it on top of the stack
                push(vm.stack[slot]);
                break;
            }

            case OP_SET_LOCAL: {
                // takes a single-byte operand for the...
                // ... stack slot where the local lives
                uint8_t slot = READ_BYTE();

                
                vm.stack[slot] = peek(0);
                break;
            }
            
            case OP_GET_GLOBAL: {
                // get name from constant table
                ObjString* name = READ_STRING();

                // global variable has never been defined
                Value value;
                if (!tableGet(&vm.globals, name, &value)) {
                    runtimeError("undefined variable '%s'", name -> chars);
                    return INTERPRET_RUNTIME_ERROR;
                }

                // otherwise push `value` onto stack
                push(value);
                break;
            }
            
            case OP_DEFINE_GLOBAL: {
                // get the name from the constant table
                ObjString* name = READ_STRING();

                // take value from top of stack and store...
                // ... in a hash table w/ name as key
                tableSet(&vm.globals, name, peek(0));

                pop();
                break;
            }

            case OP_SET_GLOBAL: {
                // get the name from the constant table
                ObjString* name = READ_STRING();

                // if variable hasn't been defined yet it's a...
                // ... runtime error to try to assign to it
                if (tableSet(&vm.globals, name, peek(0))) {
                    tableDelete(&vm.globals, name);
                    runtimeError("undefined variable '%s'", name -> chars);
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            
            case OP_EQUAL: {
                Value b = pop();
                Value a = pop();
                push(BOOL_VAL(valuesEqual(a, b)));
                break;
            }

            case OP_NOT_EQUAL: {
                Value b = pop();
                Value a = pop();
                push(BOOL_VAL(!valuesEqual(a, b)));
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

            case OP_JUMP: {
                // reads 16-bit jump offset operand
                uint16_t offset = READ_SHORT();

                // apply jmp offset to ip
                vm.ip += offset;
                break;
            }

            case OP_JUMP_IF_FALSE: {
                // reads 16-bit jump offset operand
                uint16_t offset = READ_SHORT();
                
                // if false, apply the jump offset to ip
                if (isFalsey(peek(0)))
                    vm.ip += offset;

                // otherwise, ip is left alone and execution...
                // ... auto proceeds to the next instruction...
                // ... following the jump instruction

                break;
            }

            case OP_LOOP: {
                uint16_t offset = READ_SHORT();
                vm.ip -= offset;
                break;
            }

            // exit the interpreter
            case OP_RETURN: {
                return INTERPRET_OK;
            }
        }
    }
}

// macros are all undeclared (i think?)
#undef READ_BYTE
#undef READ_SHORT
#undef READ_CONSTANT
#undef READ_STRING
#undef BINARY_OP

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
