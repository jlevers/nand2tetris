#ifndef _VM_CONSTANTS_H
#define _VM_CONSTANTS_H

// Types of VM operations
extern const char *PUSH_OP;
extern const char *POP_OP;
extern const char *LABEL_OP;
extern const char *GOTO_OP;
extern const char *IF_OP;
extern const char *FUNCTION_OP;
extern const char *CALL_OP;
extern const char *RETURN_OP;
extern const char *ARITHMETIC_OPS[10];

extern const int ARITHMETIC_OPS_NUM_ARGS[9];
extern const int SHORTEST_OP_LEN;

extern const char LABEL_SEPARATOR;
// FUNCTION_SEPARATOR has to use #define so that it can be used in the definition of LABEL_CHAR_RANGES
// without causing "initializer element is not constant" compilation error
#define FUNCTION_SEPARATOR '.'

extern const char LABEL_CHAR_RANGES[5][2];

extern const char *DEFAULT_FUNC_NAME;

#endif /* _VM_CONSTANTS_H */
