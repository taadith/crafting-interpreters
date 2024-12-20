#ifndef clox_common_h
#define clox_common_h

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// use existing "debug" mode to print out the chunk's bytecode
#define DEBUG_PRINT_CODE

// dynamic debugging is enabled for VM
#define DEBUG_TRACE_EXECUTION

// #define DEBUG_CONTROL_FLOW

#define UINT8_COUNT (UINT8_MAX + 1)

#endif
