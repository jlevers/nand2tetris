/*
 * Encoder for the nand2tetris assembler.
 * @author Jesse Evers
 * @email jesse27999@gmail.com
 * @created 5/24/19
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "encoder.h"

#ifndef _ENCODER_VARS
#define _ENCODER_VARS

char const MEM = 'M';
const int COMP_CODE_LEN = 6;

char *destinations[][6] = {
	{"M\0"},
	{"D\0"},
	{"MD\0", "DM\0"},
	{"A\0"},
	{"AM\0", "MA\0"},
    {"AD\0", "DA\0"},
    {"AMD\0", "ADM\0", "MAD\0", "MDA\0", "DMA\0", "DAM\0"}
};
char *destination_codes[] = {
	"001\0",
	"010\0",
	"011\0",
	"100\0",
	"101\0",
	"110\0",
	"111\0"
};

char *computations[][2] = {
    {"0\0"},
	{"1\0"},
	{"-1\0"},
	{"D\0"},
	{"A\0", "M\0"},
	{"!A\0", "!M\0"},
    {"!D\0"},
	{"-D\0"},
	{"-A\0", "-M\0"},
	{"D+1\0"},
    {"A+1\0", "M+1\0"},
	{"D-1\0"},
	{"A-1\0", "M-1\0"},
	{"D+A\0", "D+M\0"},
	{"D-A\0", "D-M\0"},
	{"A-D\0", "M-D\0"},
    {"D&A\0", "D&M\0"},
	{"D|A\0", "D|M\0"}
};
char *computation_codes[] = {
    "101010\0",
	"111111\0",
	"111010\0",
	"001100\0",
	"110000\0",
	"001101\0",
	"110001\0",
	"001111\0",
	"110011\0",
    "011111\0",
	"110111\0",
	"001110\0",
	"110010\0",
	"000010\0",
	"010011\0",
	"000111\0",
	"000000\0",
	"010101\0"
};

char *jumps[] = {
	"JGT\0",
	"JEQ\0",
	"JGE\0",
	"JLT\0",
	"JNE\0",
	"JLE\0",
	"JMP\0"
};
// The jump codes are the same as the destination codes
char **jump_codes = destination_codes;

#endif


/**
 * Converts a destination command string from the parser, into the machine code for that
 * destination command.
 * @param  comp_str The destination command to convert.
 * @return          The machine code for the given destination command.
 */
char* encode_dest(char* dest_str) {
    if (!dest_str) {
        return "000";
    }

    for (int i = DEST_M; i < DEST_AMD; i++) {
        char** dests_to_match = destinations[i];
        int inner = sizeof(dests_to_match);
        for (int j = 0; j < inner; j++) {
            if (dests_to_match[j] == NULL) {
                j = inner;
            } else {
                if (!strcmp(dest_str, dests_to_match[j])) {
                    return destination_codes[i];
                }
            }
        }
    }

    printf("Invalid destination `%s`\n", dest_str);
    exit(EXIT_FAILURE);
}

/**
 * Converts a computation command string from the parser, into the machine code for that
 * computation command.
 * @param  comp_str The computation command to convert.
 * @return          The machine code for the given computation command.
 */
char* encode_comp(char* comp_str) {
    char* comp = malloc(sizeof(char));
    comp[0] = '\0';
    for (int i = COMP_ZERO; i < COMP_D_OR_A_M; i++) {
        char** comps_to_match = computations[i];
        int inner = sizeof(comps_to_match);
        for (int j = 0; j < inner; j++) {
            if (comps_to_match[j] == NULL) {
                j = inner;
            } else if (!strcmp(comp_str, comps_to_match[j])) {
                comp = realloc(comp, (COMP_CODE_LEN + 2) * sizeof(char));
                strcpy(comp + 1, computation_codes[i]);
                comp[strlen(comp) - 1] = '\0';
            }
        }
    }

    if (!sizeof(comp)) {
        printf("Invalid computation `%s`\n", comp_str);
        exit(EXIT_FAILURE);
    }

    char comp_type = '0';
    if (strchr(comp_str, MEM) != NULL) {
        comp_type = '1';
    }
    comp[0] = comp_type;
    return comp;
}

/**
 * Converts a jump command string from the parser, into the machine code for that jump command.
 * @param  jump_str The jump command to convert.
 * @return          The machine code for the given jump command.
 */
char* encode_jump(char* jump_str) {
    if (!jump_str) {
        return "000";
    }

    for (int i = JUMP_GT; i < JUMP_ABS; i++) {
        if (!strcmp(jump_str, jumps[i])) {
            return jump_codes[i];
        }
    }

    printf("Invalid jump command `%s`\n", jump_str);
    exit(EXIT_FAILURE);
}
