#include <stdarg.h>
#include <stdio.h>

#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "vm.h"

VM vm;

// example of a variadic function
static void runtimeError(const char* format,...) {
    // `args` lets us pass an arbitrary...
    // ... # of args to runTime error
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    fputs("\n", stderr);

    size_t instruction = vm.ip - vm.chunk -> code - 1;
    // int line = vm.chunk -> lines[instruction];
    int line = getLine(vm.chunk, instruction);
    fprintf(stderr, "[line %d] in script\n", line);
    
    // "resetting" the stack
    vm.count = 0;
}

// initializes a VM
void initVM(void) {
    vm.count = 0;
    vm.capacity = 0;
    vm.dyn_stack = NULL;
}

// frees a VM
void freeVM(void) {
    vm.count = 0;
    vm.capacity = 0;

    NEW_FREE_ARRAY(vm.dyn_stack);
}

// pushes a Value to the stack
void push(Value value) {
    if (vm.capacity < vm.count + 2) {
        int oldCapacity = vm.capacity;
        vm.capacity = GROW_CAPACITY(oldCapacity);

        vm.dyn_stack = NEW_GROW_ARRAY(Value, vm.dyn_stack,
            vm.capacity);
    }

    vm.dyn_stack[vm.count] = value;
    vm.count++;
}

// pops a Value off the stack
Value pop(void) {
    // change where top of stack is
    vm.count--;

    return vm.dyn_stack[vm.count];
}

// returns a Value from the stack, without popping
Value peek(int distance) {
    return vm.dyn_stack[vm.count - distance - 1];
}

// beating heart of VM..
// ... interpreter spends ~90% of time here
static InterpretResult run(void) {
    // reads byte currently pointed @ by `ip`...
    // ... and then advances `ip`
    #define READ_BYTE() (*(vm.ip)++)

    // reads next byte from bytecode, treating...
    // ... resulting # as an index, and looks up the...
    // ... corresponding Value in the chunk's constant table
    #define READ_CONSTANT() (vm.chunk -> \
        constants.values[READ_BYTE()])

    // checks that both operands are numbers, then we pop...
    // ... and unwrap them; then we apply the given operator...
    // ... wrap the result, and push it back on the stack
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
        // diagnostic logging for VM...
        // ... stack trace...
        // ... and disassembling instructions
        #ifdef DEBUG_TRACE_EXECUTION
        printf("\t\t");
        for(int i = 0; i < vm.count; i++) {
            printf("[ ");
            printValue(vm.dyn_stack[i]);
            printf(" ]");
        }
        printf("\n");

        disassembleInstructionWithRLE(vm.chunk,
            (int) (vm.ip - vm.chunk -> code));
        #endif

        uint8_t instruction;
        switch (instruction = READ_BYTE()) {
            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                push(constant);
                break;
            }

            case OP_CONSTANT_LONG: {
                int first_byte_index = READ_BYTE();
                int second_byte_index = READ_BYTE();
                int third_byte_index = READ_BYTE();
                
                int constant_index = (first_byte_index << 16) |
                         (second_byte_index << 8) |
                         (third_byte_index);

                Value constant = vm.chunk -> 
                    constants.values[constant_index];
                push(constant);
                break;
            }

            case OP_ADD: {
                BINARY_OP(NUMBER_VAL, +);
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

            case OP_NEGATE: {
                // if the Value on top of the stack...
                // ... isn't a number, then we we report...
                // ... it as a runtime error and stop the...
                // ... interpreter
                if (!IS_NUMBER(peek(0))) {
                    runtimeError("operand must be a number");
                    return INTERPRET_RUNTIME_ERROR;
                }
                
                // pop the operand, unwrap it, negate it,...
                // ... wrap the result, and then push it
                push(NUMBER_VAL(-AS_NUMBER(pop())));
                break;
            }

            case OP_RETURN: {
                printValue(pop());
                printf("\n");
                return INTERPRET_OK;
            }
        }
    }

    #undef READ_BYTE
    #undef READ_CONSTANT
    #undef BINARY_OP
}

InterpretResult interpret(const char* src) {
    // create an empty chunk that's passed...
    // ... over to the compiler
    Chunk chunk;
    initChunk(&chunk);

    // compiler fills up chunk with bytecode...
    // ... unless there are compile errors
    if (!compile(src, &chunk)) {
        freeChunk(&chunk);
        return INTERPRET_COMPILE_ERROR;
    }

    vm.chunk = &chunk;
    vm.ip = vm.chunk -> code;

    InterpretResult res = run();

    freeChunk(&chunk);
    
    return res;
}
