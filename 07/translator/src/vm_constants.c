#include <stdlib.h>

#ifndef _VM_CONSTANTS
#define _VM_CONSTANTS

const char *ARITHMETIC_OPS[10] = {"add", "sub", "neg", "eq", "gt", "lt", "and", "or", "not", NULL};
const char *PUSH_OP = "push";
const char *POP_OP = "pop";
const char *LABEL_OP = "label";
const char *GOTO_OP = "goto";
const char *IF_OP = "if-goto";
const char *FUNCTION_OP = "function";
const char *CALL_OP = "call";
const char *RETURN_OP = "return";

const int ARITHMETIC_OPS_NUM_ARGS[9] = {2, 2, 1, 2, 2, 2, 2, 2, 1};
const int SHORTEST_OP_LEN = 2;

#endif