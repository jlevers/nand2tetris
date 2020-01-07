/*
 * Encoder for the nand2tetris assembler.
 * @author Jesse Evers
 * @email jesse27999@gmail.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "encoder.h"

#ifndef _ENCODER_VARS
#define _ENCODER_VARS

const char MEM = 'M';
const int COMP_CODE_LEN = 6;

/*
 * Note that while this assembler supports giving multi-symbol destinations in any order,
 * the CPUEmulator.sh program supplied with the Nand2Tetris course only allows multiple destinations
 * when given in a specific order (e.g., CPUEmulator.sh supports the destination "MD", but not "DM").
 * That means that if you're testing your Hack programs using CPUEmulator.sh, you will get the error
 * "In line XXX, Destination expected" if you use an alternate multi-destination code.
 */
const char *DESTINATIONS[][6] = {
	{"M"},
	{"D"},
	{"MD", "DM"},
	{"A"},
	{"AM", "MA"},
    {"AD", "DA"},
    {"AMD", "ADM", "MAD", "MDA", "DMA", "DAM"}
};
int DESTINATION_LENGTHS[7] = {1, 1, 2, 1, 2, 2, 6};
const char *DESTINATION_CODES[] = {
	"001",
	"010",
	"011",
	"100",
	"101",
	"110",
	"111"
};

const char *COMPUTATIONS[][4] = {
    {"0"},
	{"1"},
	{"-1"},
	{"D"},
	{"A", "M"},
	{"!D"},
	{"!A", "!M"},
	{"-D"},
	{"-A", "-M"},
	{"D+1", "1+D"},
    {"A+1", "1+A", "M+1", "1+M"},
	{"D-1"},
	{"A-1", "M-1"},
	{"D+A", "A+D", "D+M", "M+D"},
	{"D-A", "D-M"},
	{"A-D", "M-D"},
    {"D&A", "A&D", "D&M", "M&D"},
	{"D|A", "A|D", "D|M", "M|D"}
};
int COMPUTATION_LENGTHS[18] = {1, 1, 1, 1, 2, 1, 2, 1, 2, 2, 4, 1, 2, 4, 2, 2, 4, 4};
const char *COMPUTATION_CODES[] = {
    "101010",
	"111111",
	"111010",
	"001100",
	"110000",
	"001101",
	"110001",
	"001111",
	"110011",
    "011111",
	"110111",
	"001110",
	"110010",
	"000010",
	"010011",
	"000111",
	"000000",
	"010101"
};

int NUM_JUMP_TYPES = 7;
const char *JUMPS[] = {
	"JGT",
	"JEQ",
	"JGE",
	"JLT",
	"JNE",
	"JLE",
	"JMP"
};
// The jump codes are the same as the destination codes
const char **jump_codes = DESTINATION_CODES;

#endif


/**
 * Converts a destination command string from the parser, into the machine code for that
 * destination command.
 * @param  comp_str The destination command to convert.
 * @return          The machine code for the given destination command.
 */
const char *encode_dest(char *dest_str) {
    if (!dest_str) {
        return "000\0";
    }

    for (unsigned int i = 0; i < sizeof(DESTINATION_LENGTHS) / sizeof(int); i++) {
        const char **dests_to_match = DESTINATIONS[i];
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
char *encode_comp(char *comp_str) {
    char *comp = calloc(COMP_CODE_LEN + 2, sizeof(char));
	// Start computation command with bit indicating command type (0 means A, 1 means M)
	char comp_type = '0';
	if (strchr(comp_str, MEM) != NULL) {
		comp_type = '1';
	}
	comp[0] = comp_type;

	// Iterate over list of all possible computations
    for (unsigned int i = 0; i < sizeof(COMPUTATION_LENGTHS) / sizeof(int); i++) {
        const char **comps_to_match = COMPUTATIONS[i];
        int len = COMPUTATION_LENGTHS[i];
		// Iterate over meanings of this command (a command can have two different meanings
		// depending on the value of comp_type)
        for (int j = 0; j < len; j++) {
            if (!strcmp(comp_str, comps_to_match[j])) {
				// Copy starting one character into the string to account for comp_type
                strcpy(comp + 1, COMPUTATION_CODES[i]);
                comp[strlen(comp)] = '\0';
                break;
            }
        }
    }

    if (sizeof(comp) <= 1) {
		free(comp);
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
const char *encode_jump(char *jump_str) {
    if (!jump_str) {
        return "000\0";
    }

    for (int i = 0; i < NUM_JUMP_TYPES; i++) {
        if (!strcmp(jump_str, JUMPS[i])) {
            return jump_codes[i];
        }
    }

    printf("Invalid jump command `%s`\n", jump_str);
    exit(EXIT_FAILURE);
}
