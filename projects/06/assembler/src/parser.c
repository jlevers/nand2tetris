/*
 * Parser for the nand2tetris assembler.
 * @author Jesse Evers
 * @email jesse27999@gmail.com
 * @created 5/22/19
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

#ifndef _PARSER_VARS
#define _PARSER_VARS

const char* FOUT_EXT = ".hack\0";

const char BEGIN_COMMENT = '/';
const char A_CMD_BEGIN = '@';
const char L_CMD_BEGIN = '(';
const char ASSIGN = '=';
const char SEP = ';';
const char EOL = '\n';

const int MAX_DEST_LEN = 3;  // The longest possible destination command is AMD
const int MAX_COMP_LEN = 3;  // The longest possible computation command is D+M, A|M, etc
const int JMP_LEN = 3;       // All jump commands are 3 characters long

#endif


/**
 * Opens the .asm file for parsing.
 * @param  filename The path to the .asm file to parse.
 * @return          The file handler pointer for the .asm file.
 */
io init(const char* file_in) {
    FILE *in = fopen(file_in, "r");

    if (!in) {
        perror("Failed to open input file");
        exit(EXIT_FAILURE);
    }

    int period_idx = -1;
    for (int i = strlen(file_in) - 1; i >= 0; i--) {
        if (file_in[i] == '.') {
            period_idx = i;
            break;
        }
    }

    // Find filename of input up until the period, if one exists, and create an output file named
    // <file_in>.hack. If there's no '.' in file_in, create an output file named out.hack.
    char* file_out;
    if (period_idx > -1) {
        file_out = calloc(period_idx + strlen(FOUT_EXT) + 1, sizeof(char));
        file_out = strncpy(file_out, file_in, period_idx);
    } else {
        char* name = "out";
        file_out = calloc(strlen(name) + strlen(FOUT_EXT) + 1, sizeof(char));
        file_out = strcpy(file_out, name);
    }
    strcat(file_out, FOUT_EXT);

    FILE *out = fopen(file_out, "w");

    if (!out) {
        perror("Failed to open output file");
        exit(EXIT_FAILURE);
    }

    io ret;
    ret.in = in;
    ret.out = out;
    return ret;
}

/**
 * Gets the next line of the .asm file being parsed.
 * @param  file  The file to get the next line of.
 * @return The next line of the file.
 */
char* advance(FILE *file) {
    int c = 0;
    size_t line_size = 1;
    int precomment = 0;
    int skip = 0;
    int inline_comment = 0;
    char* command = calloc(1, sizeof(char));
    command[0] = '\0';
    while ((c = fgetc(file)) != EOF) {
        if (!inline_comment) {  // If we've reached an inline comment, just keep reading till EOL
            if (c == BEGIN_COMMENT) {  // Skip comments
                if (precomment) {
                    if (line_size > 2) {     // Already read more than just '//',
                        inline_comment = 1;  // so this must be an inline comment
                    } else {
                        skip = 1;
                    }
                } else {
                    precomment = 1;
                }
            } else if (c == EOL && strlen(command) <= 1) {  // Skip empty lines
                skip = 1;
            } else if (c == ' ' || c == '\t') {  // Skip whitespace
                continue;
            }

            line_size++;
            command = realloc(command, sizeof(char) * line_size);
            command[line_size - 2] = c;
            command[line_size - 1] = '\0';
        }

        if (c == EOL) {
            int end_offset = inline_comment ? 2 : 1;
            command[(line_size - 1) - end_offset] = '\0';
            break;
        }
    }


    if (c == EOF) {
        if (ferror(file)) {
            perror("Error advancing to next line of .asm file");
            exit(EXIT_FAILURE);
        }
        return NULL;
    }

    if (skip || strlen(command) <= 1) {
        return advance(file);
    }
    return command;
}

/**
 * Determines the command type of @command.
 * @param  command The command to determine the type of.
 * @return         The command type -- one of A_COMMAND, C_COMMAND, or L_COMMAND
 */
command_t command_type(const char* command) {
    if (command[0] == A_CMD_BEGIN) {
        return A_COMMAND;
    } else if (command[0] == L_CMD_BEGIN) {
        return L_COMMAND;
    }

    return C_COMMAND;
}

/**
 * Parses a symbol out of a A_COMMAND or L_COMMAND.
 * @param  symbol_type One of A_COMMAND or L_COMMAND -- the type of command to parse.
 * @param  command     The command to parse the symbol out of.
 * @return             The symbol in the current line.
 */
char* parse_symbol(command_t symbol_type, char* command) {
    if (symbol_type == A_COMMAND) {
        // In the case of an A_COMMAND, the symbol is denoted @Xxx, so the length of the symbol is
        // the length of the command minus the length of the '@'.
        int sym_len = strlen(command) - 1;
        char *cmd = calloc(sym_len + 1, sizeof(char));
        strncpy(cmd, command + 1, sym_len);
        cmd[sym_len] = '\0';
        return cmd;
    }

    // In the case of an L_COMMAND, the symbol is denoted `(Xxx)`, so the length of the symbol is
    // the length of the command, `(Xxx)`, minus the length of the parentheses, `()`.
    int sym_len = strlen(command) - 2;
    char* sym_name = calloc(sym_len + 1, sizeof(char));
    for (int i = 0; i < sym_len; i++) {
        sym_name[i] = command[i + 1];
    }

    sym_name[sym_len] = '\0';
    return sym_name;
}

/**
 * Parses out the destination(s) in which to store the result of the current computation, if a
 * destination exists.
 * @param  command  The command to parse the symbol out of.
 * @return The destination(s) in which to store the result of the current computation, or NULL if no
 *      destinations were given in the current command.
 */
char* parse_dest(const char* command) {
    char* destination = calloc(1, sizeof(char));
    destination[0] = '\0';
    for (int i = 0; i < MAX_DEST_LEN + 1; i++) {
        if (command[i] == ASSIGN) {  // Destinations are followed by an assignment symbol, =
            return destination;
        // If a separator or newline is reached, the command doesn't include a destination
        } else if (command[i] == SEP || command[i] == EOL) {
            return NULL;
        }

        destination = realloc(destination, sizeof(char) * (i + 2));
        destination[i] = command[i];
        destination[i + 1] = '\0';
    }

    return NULL;
}

/**
 * Parses out the computation from the current command.
 * @param  command  The command to parse the symbol out of.
 * @return The computation from the current command.
 */
char* parse_comp(const char* command) {
    int start_comp_idx = 0;
    int end_comp_idx = 0;

    int i = 0;
    for (; i < strlen(command); i++) {
        if (command[i] == ASSIGN) {
            start_comp_idx = i + 1;
        } else if (command[i] == SEP) {
            end_comp_idx = i;
        }
    }

    // This is when there's no jump statement, so the computation statement goes to EOL
    if (!end_comp_idx) {
        end_comp_idx = i;
    }

    // Copy the computation text from the command
    int comp_len = end_comp_idx - start_comp_idx;
    char* computation = calloc(comp_len + 1, sizeof(char));
    for (int i = 0; i < comp_len; i++) {
        computation[i] = command[i + start_comp_idx];
    }
    computation[comp_len] = '\0';
    return computation;
}

/**
 * Parses out the jump statement, if any, in the current command.
 * @param  command  The command to parse the symbol out of.
 * @return The current jump command if one exists, NULL otherwise.
 */
char* parse_jump(const char* command) {
    int command_len = strlen(command);
    int i = 0;
    for (; i < command_len; i++) {
        if (command[i] == EOL) {
            return NULL;
        } else if (command[i] == SEP) {  // Jump commands always start after a separator (";")
            i++;
            break;
        }
    }

    if (command_len == i) {
        return NULL;
    }

    char* jump = calloc(JMP_LEN + 1, sizeof(char));
    memcpy(jump, command + i, JMP_LEN * sizeof(char));
    jump[strlen(jump)] = '\0';
    return jump;
}
