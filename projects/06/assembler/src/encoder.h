/*
 * Header file for the encoder for the nand2tetris assembler.
 * @author Jesse Evers
 * @email jesse27999@gmail.com
 */

#ifndef _ENCODER_H
#define _ENCODER_H

extern const char MEM;  // Character used to reference a memory address in .asm files
extern const int COMP_CODE_LEN;  // The max length of a binary computation command (in a .hack file)

extern const char *DESTINATIONS[][6];    // An array of .asm style destinations
extern int DESTINATION_LENGTHS[7];       // The respective lengths of each of the above destinations
extern const char *DESTINATION_CODES[];  // An array of binary destination codes, corresponding to DESTINATIONS
extern const char *COMPUTATIONS[][4];    // An array of all possible .asm-style computation commands
extern int COMPUTATION_LENGTHS[18];      // The respective number of characters in each of the above commands
extern const char *COMPUTATION_CODES[];  // The binary equivalents for each computation command in COMPUTATIONS
extern int NUM_JUMP_TYPES;               // The number of possible .asm-style jump commands
extern const char *JUMPS[];              // A list of all the .asm-style jump commands


const char *encode_dest(char*);
char *encode_comp(char*);
const char *encode_jump(char*);

#endif
