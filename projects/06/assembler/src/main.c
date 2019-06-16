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
 * @param  num The number to convert to binary.
 * @return     A string containing the binary version of @num.
 */
char* parse_to_binary(int num) {
    char *binary = malloc(WORD * sizeof(char));
    memset(binary, '0', WORD);

    int mult = pow(2, WORD - 2);
    for (int i = 1; i < WORD; i++) {
        if (num <= 0) {
            return binary;
        }

        if (num >= mult) {
            num -= mult;
            binary[i] = '1';
        }

        mult /= 2;
    }

    if (num > 0) {
        printf("parse_to_binary only parses integers <= 2^%d - 1.\n", WORD);
        exit(EXIT_FAILURE);
    }

    return binary;
}

void process_by_lines(FILE *in, FILE *out, ht_hash_table *ht,
    void (*func)(FILE*, FILE*, char*, ht_hash_table*)) {
    // Keep going until there's a null command (i.e., EOF)
    while (1) {
        char* command = advance(in);
        if (!command) {
            break;
        }
        (*func)(in, out, command, ht);
    }
}

void first_pass(FILE *in, FILE *out, char *command, ht_hash_table *ht) {
    command_t cmd_type;
    char* destination;
    char* computation;
    char* jump_to;
    char* dest_encoded;
    char* comp_encoded;
    char* jump_encoded;
    char* binary_addr;

    cmd_type = command_type(command);
    // Add 1 to command length for newline at end
    char* cmd_out = malloc((WORD + 1) * sizeof(char));

    if (cmd_type == C_COMMAND) {
        // Parse command
        destination = parse_dest(command);
        computation = parse_comp(command);
        jump_to = parse_jump(command);

        // Encode command
        dest_encoded = encode_dest(destination);
        comp_encoded = encode_comp(computation);
        jump_encoded = encode_jump(jump_to);

        // Generate machine code
        strcat(cmd_out, "111");
        strcat(cmd_out, comp_encoded);
        strcat(cmd_out, dest_encoded);
        strcat(cmd_out, jump_encoded);
    } else {  // Just convert the input to an address
        binary_addr = parse_to_binary(atoi(command + 1));
        strcat(cmd_out, binary_addr);
    }

    // Add a newline to the end of the machine instruction and write it to file
    cmd_out[strlen(cmd_out)] = '\n';
    fwrite(cmd_out, sizeof(char), WORD + 1, out);
    free(command);
}

void second_pass(FILE *in, FILE *out, char *command, ht_hash_table *ht) {

}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: ./assembler path/to/prog.asm");
        return EXIT_FAILURE;
    }

    io files = init(argv[1]);
    FILE *in = files.in;
    FILE *out = files.out;

    ht_hash_table *ht = constructor();

    process_by_lines(in, out, ht, first_pass);

    fclose(in);
    fclose(out);
    return 0;
}
