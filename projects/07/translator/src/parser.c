#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "parser.h"

#ifndef _VM_PARSER_VARS
#define _VM_PARSER_VARS

const char BEGIN_COMMENT = '/';
const char EOL = '\n';

const char *ARITHMETIC_CMDS[9] = { "add", "sub", "neg", "eq", "gt", "lt", "and", "or", "not" };
const char *PUSH_CMD = "push";
const char *POP_CMD = "pop";
const char *LABEL_CMD = "label";
const char *GOTO_CMD = "goto";
const char *IF_CMD = "if-goto";
const char *FUNCTION_CMD = "function";
const char *CALL_CMD = "call";
const char *RETURN_CMD = "return";

const int SHORTEST_CMD_LEN = 2;

#endif


/**
 * Opens the .vm file for processing.
 *
 * @param  char*  input_path The path to the .vm file to process
 * @return FILE*             The file handler pointer for the .vm file
 */

FILE *VM_Parser(char *input_path) {
    FILE *input = fopen(input_path, "r");

    if (!input) {
        perror("Failed to open input file");
        return NULL;
    }

    return input;
}

/**
 * Gets the next line of the .vm file being parsed.
 *
 * @param  FILE*  file  The file to get the next line of
 * @return char*        The next line of the file
 */
char *vm_advance(FILE *file) {
    int c = 0;
    size_t line_size = 0;
    int skip = 0;

    // Keep track of inline comments
    int precomment = 0;
    int inline_comment = 0;

    // Keep track of spaces to keep and get rid of
    int space = 0;
    int remove_space = 0;
    char *command = calloc(1, sizeof(char));
    command[0] = '\0';

    while ((c = fgetc(file)) != EOF) {
        if (!inline_comment) {  // If we've reached an inline comment, just keep reading till EOL
            if (c == BEGIN_COMMENT) {  // Skip comments
                if (precomment) {
                    if (line_size > 1) {     // Already read more than just '/',
                        inline_comment = 1;  // so this must be an inline comment
                        remove_space = space;
                    } else {
                        skip = 1;
                    }
                } else {
                    precomment = 1;
                }
            } else if (c == EOL && strlen(command) <= 1) {  // Skip empty lines
                skip = 1;
            } else if (c == '\t' || (c == ' ' && space)) {  // Skip tabs and consecutive spaces
                continue;
            } else if (c == ' ') {
                space = 1;
            } else {
                space = 0;
            }

            line_size++;
            command = realloc(command, line_size + 1);
            command[line_size - 1] = c;
            command[line_size] = '\0';
        }

        if (c == EOL) {
            int end_offset = remove_space + (inline_comment ? 2 : 1);
            command[line_size - end_offset] = '\0';
            break;
        }
    }

    if (c == EOF) {
        if (ferror(file)) {
            perror("Error advancing to next line of .vm file");
            free(command);
            return NULL;
        }
        free(command);
        return NULL;
    }

    if (skip || strlen(command) <= 1) {
        free(command);
        return vm_advance(file);
    }

    return command;
}


/**
 * Determines the command type of @command.
 *
 * @param  char*  command  The command to determine the type of
 * @return vm_command_t    The command type
 */
vm_command_t vm_command_type(char *line) {
    unsigned int i = 0;
    for (; i < strlen(line); i++) {
        if (line[i] == ' ') break;
    }

    if ((int)i < SHORTEST_CMD_LEN) return C_INVALID;

    // Copy the command from the VM program line given
    char *cmd = calloc(i, sizeof(char));
    cmd = strncpy(cmd, line, i);
    cmd[i] = '\0';

    vm_command_t cmd_type = C_INVALID;

    if (!strcmp(cmd, PUSH_CMD)) {
        cmd_type = C_PUSH;
    } else if (!strcmp(cmd, POP_CMD)) {
        cmd_type = C_POP;
    } else if (!strcmp(cmd, LABEL_CMD)) {
        cmd_type = C_LABEL;
    } else if (!strcmp(cmd, GOTO_CMD)) {
        cmd_type = C_GOTO;
    } else if (!strcmp(cmd, IF_CMD)) {
        cmd_type = C_IF;
    } else if (!strcmp(cmd, FUNCTION_CMD)) {
        cmd_type = C_FUNCTION;
    } else if (!strcmp(cmd, CALL_CMD)) {
        cmd_type = C_CALL;
    } else if (!strcmp(cmd, RETURN_CMD)) {
        cmd_type = C_RETURN;
    } else {
        for (i = 0; i < sizeof(ARITHMETIC_CMDS) / sizeof(ARITHMETIC_CMDS[0]); i++) {
            if (!strcmp(cmd, ARITHMETIC_CMDS[i])) {
                cmd_type = C_ARITHMETIC;
            }
        }
    }

    return cmd_type;
}


/**
 * Returns the first argument of the given command. In the case of C_ARITHMETIC, the command itself
 * ("add", "sub", etc.) is returned. Should not be called for C_RETURN. If given an invalid command
 * type, returns NULL.
 *
 * @param  char*  line  The line from the VM program
 * @return char*        The first argument, or NULL if an invalid command type is given
*/
char *vm_arg1(char *line) {
    vm_command_t cmd_type = vm_command_type(line);

    if (cmd_type == C_RETURN || cmd_type == C_INVALID) {
        return NULL;
    } else if (cmd_type == C_ARITHMETIC) {
        // Copy the original line to a new variable so that we don't return a direct pointer to the
        // `line` parameter
        char *copy = calloc(strlen(line), sizeof(char));
        strcpy(copy, line);
        return copy;
    }

    int i = 0;
    while(line[i] != ' ') { i++; }
    i++;

    int j = i;
    while(j < (int)strlen(line) && line[j] != ' ') { j++; }

    char *arg = calloc(j - i + 1, sizeof(char));
    strncpy(arg, line + i, j - i);
    arg[j - i] = '\0';

    return arg;
}


/**
 * Returns the second argument of the current command. Should be called only if the current command
 * is C_PUSH, C_POP, C_FUNCTION, or C_CALL. If called on an invalid command type, returns -1.
 *
 * @param  char*  line  The VM program line to parse the second argument out of
 * @return int          The second argument, or -1 if the second argument doesn't exist
 */
int vm_arg2(char *line) {
    vm_command_t cmd_type = vm_command_type(line);

    if (cmd_type != C_PUSH && cmd_type != C_POP && cmd_type != C_FUNCTION && cmd_type != C_CALL) {
        return -1;
    }

    int num_spaces = 0;
    int i = 0;

    // We want to go past the first two spaces in this line of the VM program. For instance:
    //      push constant 2
    //          ^        ^
    while (i < (int)strlen(line)) {
        if (line[i] == ' ') num_spaces++;
        i++;
        if (num_spaces == 2) break;
    }

    return atoi(line + i);
}
