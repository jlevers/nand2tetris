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

char *DESTINATIONS[][6] = {
	{"M\0"},
	{"D\0"},
	{"MD\0", "DM\0"},
	{"A\0"},
	{"AM\0", "MA\0"},
    {"AD\0", "DA\0"},
    {"AMD\0", "ADM\0", "MAD\0", "MDA\0", "DMA\0", "DAM\0"}
};
int DESTINATION_LENGTHS[7] = {1, 1, 2, 1, 2, 2, 6};
char *DESTINATION_CODES[] = {
	"001\0",
	"010\0",
	"011\0",
	"100\0",
	"101\0",
	"110\0",
	"111\0"
};

char *COMPUTATIONS[][2] = {
    {"0\0"},
	{"1\0"},
	{"-1\0"},
	{"D\0"},
	{"A\0", "M\0"},
	{"!D\0"},
	{"!A\0", "!M\0"},
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
int COMPUTATION_LENGTHS[18] = {1, 1, 1, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 2, 2, 2, 2, 2};
char *COMPUTATION_CODES[] = {
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

int NUM_JUMP_TYPES = 7;
char *JUMPS[] = {
	"JGT\0",
	"JEQ\0",
	"JGE\0",
	"JLT\0",
	"JNE\0",
	"JLE\0",
	"JMP\0"
};
// The jump codes are the same as the destination codes
char **jump_codes = DESTINATION_CODES;

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

    for (int i = 0; i < sizeof(DESTINATION_LENGTHS) / sizeof(int); i++) {
        char** dests_to_match = DESTINATIONS[i];
        int len = DESTINATION_LENGTHS[i];
        for (int j = 0; j < len; j++) {
            if (!strcmp(dest_str, dests_to_match[j])) {
                return DESTINATION_CODES[i];
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
    char* comp = calloc(1, sizeof(char));
	// Start computation command with bit indicating command type (0 means A, 1 means M)
	char comp_type = '0';
	if (strchr(comp_str, MEM) != NULL) {
		comp_type = '1';
	}
	comp[0] = comp_type;

	// Iterate over list of all possible computations
    for (int i = 0; i < sizeof(COMPUTATION_LENGTHS) / sizeof(int); i++) {
        char** comps_to_match = COMPUTATIONS[i];
        int len = COMPUTATION_LENGTHS[i];
		// Iterate over meanings of this command (a command can have two different meanings
		// depending on the value of comp_type)
        for (int j = 0; j < len; j++) {
            if (!strcmp(comp_str, comps_to_match[j])) {
				// Add two extra chars for comp_type and the null terminator
                comp = realloc(comp, (COMP_CODE_LEN + 2) * sizeof(char));
                strcpy(comp + 1, COMPUTATION_CODES[i]);
                comp[strlen(comp)] = '\0';
                break;
            }
        }
    }

    if (sizeof(comp) <= 1) {
        printf("Invalid computation `%s`\n", comp_str);
        exit(EXIT_FAILURE);
    }

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

    for (int i = 0; i < NUM_JUMP_TYPES; i++) {
        if (!strcmp(jump_str, JUMPS[i])) {
            return jump_codes[i];
        }
    }

    printf("Invalid jump command `%s`\n", jump_str);
    exit(EXIT_FAILURE);
}
