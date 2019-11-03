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

FILE *Parser(char*);
int has_more_commands();
char *vm_advance(FILE*);
vm_command_t vm_command_type(char*);
char *arg1(char*);
int arg2(char*);

#endif