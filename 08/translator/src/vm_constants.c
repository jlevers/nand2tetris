#include <stdlib.h>

#include "vm_constants.h"

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

const char LABEL_SEPARATOR = ':';
// There's also a FUNCTION_SEPARATOR, but it's defined as a macro in vm_constants.h for reasons
// that are explained there

// Each inner array is a range of characters that are allowed in label names
const char LABEL_CHAR_RANGES[5][2] = {
    {'0', '9'},
    {'a', 'z'},
    {'A', 'Z'},
    {'_', '_'},
    {FUNCTION_SEPARATOR, FUNCTION_SEPARATOR}
};

const char *DEFAULT_FUNC_NAME = "Main.main";
