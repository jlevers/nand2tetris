/*
 * Parser for the nand2tetris assembler.
 * @author Jesse Evers
 * @email jesse27999@gmail.com
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "encoder.h"
#include "parser.h"
#include "../../../lib/hash_table.h"

#ifndef _PARSER_VARS
#define _PARSER_VARS

const char *FOUT_EXT = ".hack\0";

const char BEGIN_COMMENT = '/';
const char A_CMD_BEGIN = '@';
const char L_CMD_BEGIN = '(';
const char ASSIGN = '=';
const char SEP = ';';
const char EOL = '\n';

const int MAX_DEST_LEN = 3;  // The longest possible destination command is AMD
const int MAX_COMP_LEN = 3;  // The longest possible computation command is D+M, A|M, etc
const int JMP_LEN = 3;       // All jump commands are 3 characters long

const int WORD = 16;

#endif


/**
 * Opens the .asm file for parsing.
 * @param  filename The path to the .asm file to parse.
 * @return          The file handler pointer for the .asm file.
 */
io init(const char *file_in) {
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
    char *file_out;
    if (period_idx > -1) {
        file_out = calloc(period_idx + strlen(FOUT_EXT) + 1, sizeof(char));
        file_out = strncpy(file_out, file_in, period_idx);
    } else {
        char *name = "out";
        file_out = calloc(strlen(name) + strlen(FOUT_EXT) + 1, sizeof(char));
        file_out = strcpy(file_out, name);
    }
    strcat(file_out, FOUT_EXT);

    FILE *out = fopen(file_out, "w");
    free(file_out);

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
char *advance(FILE *file) {
    int c = 0;
    size_t line_size = 0;
    int precomment = 0;
    int skip = 0;
    int inline_comment = 0;
    char *command = calloc(1, sizeof(char));
    command[0] = '\0';

    while ((c = fgetc(file)) != EOF) {
        if (!inline_comment) {  // If we've reached an inline comment, just keep reading till EOL
            if (c == BEGIN_COMMENT) {  // Skip comments
                if (precomment) {
                    if (line_size > 1) {     // Already read more than just '//',
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
            command = realloc(command, line_size + 1);
            command[line_size - 1] = c;
            command[line_size] = '\0';
        }

        if (c == EOL) {
            int end_offset = inline_comment ? 2 : 1;
            command[line_size - end_offset] = '\0';
            break;
        }
    }

    if (c == EOF) {
        if (ferror(file)) {
            perror("Error advancing to next line of .asm file");
            free(command);
            exit(EXIT_FAILURE);
        }
        free(command);
        return NULL;
    }

    if (skip || strlen(command) <= 1) {
        free(command);
        return advance(file);
    }

    return command;
}

/**
 * Determines the command type of @command.
 * @param  command The command to determine the type of.
 * @return         The command type -- one of A_COMMAND, C_COMMAND, or L_COMMAND
 */
command_t command_type(const char *command) {
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
char *parse_symbol(command_t symbol_type, char *command) {
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
    char *sym_name = calloc(sym_len + 1, sizeof(char));
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
 *         destinations were given in the current command.
 */
char *parse_dest(const char *command) {
    char *destination = calloc(1, sizeof(char));
    destination[0] = '\0';
    for (int i = 0; i < MAX_DEST_LEN + 1; i++) {
        if (command[i] == ASSIGN) {  // Destinations are followed by an assignment symbol, =
            return destination;
        // If a separator or newline is reached, the command doesn't include a destination
        } else if (command[i] == SEP || command[i] == EOL) {
            free(destination);
            return NULL;
        }

        destination = realloc(destination, i + 2);
        destination[i] = command[i];
        destination[i + 1] = '\0';
    }

    free(destination);
    return NULL;
}

/**
 * Parses out the computation from the current command.
 * @param  command  The command to parse the symbol out of.
 * @return The computation from the current command.
 */
char *parse_comp(const char *command) {
    int start_comp_idx = 0;
    int end_comp_idx = 0;

    unsigned int i = 0;
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
    char *computation = calloc(comp_len + 1, sizeof(char));
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
char *parse_jump(const char *command) {
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

    char *jump = calloc(JMP_LEN + 1, sizeof(char));
    memcpy(jump, command + i, JMP_LEN);
    jump[strlen(jump)] = '\0';
    return jump;
}

/**
 * Converts @num into a WORD-1 bit binary number, with a leading 0. If the number is larger than
 * 2^(WORD-1) - 1, it exits with code EXIT_FAILURE.
 *
 * @param  num  The number to convert to binary.
 * @return      A string containing the binary version of @num.
 */
char *parse_to_binary(int num) {
    char *binary = calloc(WORD + 1, sizeof(char));
    binary[WORD] = '\0';

    int mult = pow(2, WORD - 1);
    for (int i = 0; i < WORD; i++) {
        if (num >= mult) {
            num -= mult;
            binary[i] = '1';
        } else {
            binary[i] = '0';
        }
        mult /= 2;
    }

    if (num > 0) {
        printf("parse_to_binary only parses integers <= 2^%d - 1.\n", WORD);
        free(binary);
        exit(EXIT_FAILURE);
    }

    binary[WORD] = '\0';
    return binary;
}

/**
 * Performs the first assembler pass on the program, generating the symbol table to be used in the
 * second pass. After this pass, the symbol table only contains symbols corresponding to L_COMMANDs,
 * i.e., labels like (INFINITE_LOOP). It does NOT contain symbols corresponding to A_COMMANDs, i.e.,
 * @foo. Symbols are stored in key-value pairs where the key is the symbol name and the value is the
 * 16-bit binary representation of the line number of that label in the program.
 *
 * The reason that L_COMMAND symbols are inserted into the symbol table in a separate pass from
 * C_COMMAND symbols is that L_COMMAND lines are ignored in the final .hack program -- they just
 * indicate where in the program to jump to when they are used as a regular symbol. For instance:
 *
 *   ... // some operations
 *   @GO_HERE
 *   ...;JGT
 *   ... // more operations
 * (GO_HERE)
 *   ... // operations to do if the JGT condition is satisfied
 *
 * In the above example, the @GO_HERE symbol will be set to the line *after* (GO_HERE), so
 * when @GO_HERE is referenced, the program jumps to the set of commands under the label (GO_HERE).
 *
 * @param in  the file containing the program to assemble
 * @param ht  the hash table to store the program's symbol table in
 */
void first_pass(FILE *in, ht_hash_table *ht) {
    command_t cmd_type;
    int addr_ROM = 0;

    int line = 1;
    char *command = NULL;
    while ((command = advance(in)) != NULL) {
        line++;

        cmd_type = command_type(command);
        if (cmd_type == L_COMMAND) {
            char *symbol = parse_symbol(L_COMMAND, command);
            char *binary_addr = parse_to_binary(addr_ROM);
            ht_insert(ht, symbol, binary_addr);
            free(symbol);
            free(binary_addr);
        } else {
            addr_ROM++;
        }
        free(command);
        command = NULL;
    }
}

/**
 * Generates the binary program based on the file containing the program being assembled, and the
 * symbol table generated in `first_pass(...)`. It also fills out the symbol table with all
 * @foo style symbols, since those are skipped in the first pass. See first_pass() for details.
 *
 * @param in   the file containing the original assembly program
 * @param out  the file to write the assembled binary to
 * @param ht   the hash table containing the symbol table generated in `first_pass(...)`
 */
void second_pass(FILE *in, FILE *out, ht_hash_table *ht) {
    command_t cmd_type;
    int addr_RAM = 16;

    char *command = NULL;
    char *computation = NULL;
    char *destination = NULL;
    char *jump_to = NULL;
    char *comp_encoded = NULL;
    const char *dest_encoded = NULL;
    const char *jump_encoded = NULL;

    while ((command = advance(in)) != NULL) {
        char *cmd_out = calloc(WORD + 1, sizeof(char));
        cmd_out[WORD] = '\0';
        cmd_type = command_type(command);

        if (cmd_type == C_COMMAND) {
            // Parse command
            computation = parse_comp(command);
            destination = parse_dest(command);
            jump_to = parse_jump(command);

            // Encode command
            comp_encoded = encode_comp(computation);
            dest_encoded = encode_dest(destination);
            jump_encoded = encode_jump(jump_to);

            // Generate machine code
            strcpy(cmd_out, "111\0");
            strcat(cmd_out, comp_encoded);
            strcat(cmd_out, dest_encoded);
            strcat(cmd_out, jump_encoded);
        } else if (cmd_type == A_COMMAND) {  // Convert the input to an address
            char *parsed = parse_symbol(cmd_type, command);
            char *binary_addr = NULL;

            char zero = '0';
            char nine = '9';
            // If the first character of the address isn't a digit
            if ((char)parsed[0] < zero || (char)parsed[0] > nine) {
                binary_addr = ht_search(ht, parsed);

                // If we haven't already stored this symbol in the symbol table, do so
                if (binary_addr == NULL) {
                    binary_addr = parse_to_binary(addr_RAM);
                    ht_insert(ht, parsed, binary_addr);
                    addr_RAM++;
                }
            } else {
                binary_addr = parse_to_binary(atoi(command + 1));
            }

            strcpy(cmd_out, binary_addr);
            free(binary_addr);
            free(parsed);
            binary_addr = NULL;
            parsed = NULL;
        } else {
            goto cleanup;
        }

        // Add a newline to the end of the machine instruction and write it to file
        cmd_out[WORD] = '\n';
        fwrite(cmd_out, sizeof(char), WORD + 1, out);

        // I know GOTOs are the root of all evil, but it seems like a more elegant solution than
        // having a cleanup function that I have to pass 5 pointers to.
        // See https://stackoverflow.com/a/24215512/3696964.
        cleanup:
            free(computation);
            free(destination);
            free(jump_to);
            free(comp_encoded);
            free(command);
            free(cmd_out);
            computation = NULL;
            destination = NULL;
            jump_to = NULL;
            comp_encoded = NULL;
            command = NULL;
            cmd_out = NULL;
    }
}
