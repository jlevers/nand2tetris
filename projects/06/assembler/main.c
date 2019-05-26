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

const int WORD = 16;

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

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: ./assembler path/to/prog.asm");
        return EXIT_FAILURE;
    }

    io files = init(argv[1]);
    FILE *in = files.in;
    FILE *out = files.out;

    command_t cmd_type;
    char* destination;
    char* computation;
    char* jump_to;
    char* dest_encoded;
    char* comp_encoded;
    char* jump_encoded;
    char* binary_addr;

    while (1) {
        char* command = advance(in);
        if (!command) {
            break;
        }
        cmd_type = command_type(command);
        char* cmd_out = malloc(WORD * sizeof(char));

        if (cmd_type == C_COMMAND) {
            destination = parse_dest(command);
            computation = parse_comp(command);
            jump_to = parse_jump(command);

            dest_encoded = encode_dest(destination);
            comp_encoded = encode_comp(computation);
            jump_encoded = encode_jump(jump_to);

            strcat(cmd_out, "111");
            strcat(cmd_out, comp_encoded);
            strcat(cmd_out, dest_encoded);
            strcat(cmd_out, jump_encoded);
        } else {
            binary_addr = parse_to_binary(atoi(command + 1));
            strcat(cmd_out, binary_addr);
        }

        cmd_out = realloc(cmd_out, (strlen(cmd_out) + 1) * sizeof(char));
        cmd_out[strlen(cmd_out) - 1] = '\n';
        fwrite(cmd_out, sizeof(char), WORD, out);
        free(command);
    }

    free(destination);
    free(computation);
    free(jump_to);
    fclose(in);
    fclose(out);
    return 0;
}
