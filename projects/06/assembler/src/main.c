/*
 * Main controller for the nand2tetris assembler.
 * @author Jesse Evers
 * @email jesse27999@gmail.com
 * @created 5/24/19
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "parser.h"
#include "encoder.h"
#include "symboltable.h"
#include "../hashtable/src/hash_table.h"

const int WORD = 16;

/**
 * Converts @num into a WORD-1 bit binary number, with a leading 0. If the number is larger than
 * 2^(WORD-1) - 1, it exits with code EXIT_FAILURE.
 *
 * @param  num The number to convert to binary.
 * @return     A string containing the binary version of @num.
 */
char* parse_to_binary(int num) {
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
 * second pass. The symbol table, when generated, has key-value pairs where the key is the symbol
 * name and the value is the line number of that symbol in the program.
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
 * symbol table generated in `first_pass(...)`.
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
            int from_symbol_table = 0;

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
                } else {
                    from_symbol_table = 1;
                }
            } else {
                binary_addr = parse_to_binary(atoi(command + 1));
            }

            strcpy(cmd_out, binary_addr);
            // Don't free binary_addr if we retrieved it from the symbol table, because we'd be
            // deleting it from the symbol table
            if (!from_symbol_table) free(binary_addr);
            free(parsed);
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

int main(int argc, char *argv[]) {
    setbuf(stdout, NULL);
    if (argc != 2) {
        printf("Usage: ./assembler path/to/prog.asm");
        return EXIT_FAILURE;
    }

    io files = init(argv[1]);
    FILE *in = files.in;
    FILE *out = files.out;

    // Count number of lines in file
    int line_count = 0;
    for (char c = getc(in); c != EOF; c = getc(in)) {
        if (c == '\n') line_count++;
    }
    fseek(in, 0, SEEK_SET);

    // 2/3rds the number of lines in the file is the maximum number of variables and labels possible
    // given the assembly syntax we're using
    ht_hash_table *ht = constructor(2 * (line_count / 3));

    first_pass(in, ht);
    fseek(in, 0, SEEK_SET);
    second_pass(in, out, ht);

    fclose(in);
    fclose(out);
    ht_delete(ht);
    return 0;
}
