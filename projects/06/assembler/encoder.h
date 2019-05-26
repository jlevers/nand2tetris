/*
 * Header file for the encoder for the nand2tetris assembler.
 * @author Jesse Evers
 * @email jesse27999@gmail.com
 * @created 5/24/19
 */

#ifndef _ENCODER_H
#define _ENCODER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Destination types, strings, and codes
enum dest_t {
	DEST_M,
	DEST_D,
	DEST_MD,
	DEST_A,
	DEST_AM,
	DEST_AD,
	DEST_AMD
};

// Computation types, strings, and codes
enum comp_t {
    COMP_ZERO,
	COMP_ONE,
	COMP_NEG_ONE,
	COMP_D,
	COMP_A_M,
	COMP_NOT_D,
	COMP_NOT_A_M,
	COMP_NEG_D,
    COMP_NEG_A_M,
	COMP_D_ADD1,
	COMP_A_M_ADD1,
	COMP_D_SUB1_,
	COMP_A_M_SUB_1,
	COMP_D_ADD_A_M,
    COMP_D_SUB_A_M,
	COMP_A_M_SUB_D,
	COMP_D_AND_A_M,
	COMP_D_OR_A_M
};

// Jump types, strings, and codes
enum jump_t {
	JUMP_GT,
	JUMP_EQ,
	JUMP_GE,
	JUMP_LT,
	JUMP_NE,
	JUMP_LE,
	JUMP_ABS
};

char* encode_dest(char*);
char* encode_comp(char*);
char* encode_jump(char*);

#endif
