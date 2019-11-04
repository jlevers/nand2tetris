#ifndef _VM_PARSER_H
#define _VM_PARSER_H

#include <stdio.h>

typedef enum vm_command_t {
    C_INVALID = -1,
    C_ARITHMETIC = 0,
    C_PUSH = 1,
    C_POP = 2,
    C_LABEL = 3,
    C_GOTO = 4,
    C_IF = 5,
    C_FUNCTION = 6,
    C_RETURN = 7,
    C_CALL = 8
} vm_command_t;


extern const char BEGIN_COMMENT;
extern const char EOL;

// Types of VM commands
extern const char *ARITHMETIC_CMDS[9];
extern const char *PUSH_CMD;
extern const char *POP_CMD;
extern const char *LABEL_CMD;
extern const char *GOTO_CMD;
extern const char *IF_CMD;
extern const char *FUNCTION_CMD;
extern const char *CALL_CMD;
extern const char *RETURN_CMD;
extern const int SHORTEST_CMD_LEN;


FILE *VM_Parser(char*);
char *vm_advance(FILE*);
vm_command_t vm_command_type(char*);
char *vm_arg1(char*);
int vm_arg2(char*);

#endif
