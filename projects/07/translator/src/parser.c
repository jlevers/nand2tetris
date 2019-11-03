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


vm_command_t vm_command_type(char *line) {
    int i = 0;
    for (; i < (int)strlen(line); i++) {
        if (line[i] == ' ' || line[i] == EOL) break;
    }

    if (i < SHORTEST_CMD_LEN) return C_INVALID;

    char *cmd = calloc(i + 1, sizeof(char));
    cmd = strncpy(cmd, line, i);
    cmd[i - 1] = '\0';

    vm_command_t cmd_type = C_INVALID;
    printf("cmd: %s\n", cmd);
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
 * Opens the .vm file for processing.
 * @param  filename The path to the .vm file to process.
 * @return          The file handler pointer for the .vm file.
 */

FILE *Parser(char *input_path) {
    FILE *input = fopen(input_path, "r");

    if (!input) {
        perror("Failed to open input file");
        return NULL;
    }

    return input;
}

/**
 * Gets the next line of the .vm file being parsed.
 * @param  file  The file to get the next line of.
 * @return The next line of the file.
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
