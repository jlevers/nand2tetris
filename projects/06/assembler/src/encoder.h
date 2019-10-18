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

const char *encode_dest(char*);
char *encode_comp(char*);
const char *encode_jump(char*);

#endif
