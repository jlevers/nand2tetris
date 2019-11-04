/*
 * Header file for the parser for nand2tetris assembler.
 * @author Jesse Evers
 * @email jesse27999@gmail.com
 */

#ifndef _PARSER_H
#define _PARSER_H

#include <stdio.h>
#include "../hashtable/src/hash_table.h"

typedef enum ct {
  A_COMMAND = 0,
  C_COMMAND = 1,
  L_COMMAND = -1
} command_t;

typedef struct io {
    FILE *in;
    FILE *out;
} io;

extern const char *FOUT_EXT;  // The file extension to use for outputted files

// All "commands" discussed below are .asm-style commands
extern const char BEGIN_COMMENT;  // The char that designates the beginning of a comment
extern const char A_CMD_BEGIN;    // The char that designates the beginning of an A_COMMAND
extern const char L_CMD_BEGIN;    // The char that designates the beginning of an L_COMMAND
extern const char ASSIGN;         // The char that indicates assignment
extern const char SEP;            // The char that separates the computation and jump portions of a command
extern const char EOL;            // The char that designates the end of a line

extern const int MAX_DEST_LEN;  // The length of the longest possible destination command
extern const int MAX_COMP_LEN;  // The length of the longest possible computation command
extern const int JMP_LEN;       // The length of a jump command

extern const int WORD;  // The length of a binary word in the output file


io init(const char*);
char *advance(FILE*);
command_t command_type(const char*);
char *symbol(command_t, const char*);
char *parse_dest(const char*);
char *parse_comp(const char*);
char *parse_jump(const char*);
char *parse_symbol(command_t, char*);
char *parse_to_binary(int);
void first_pass(FILE*, ht_hash_table*);
void second_pass(FILE*, FILE*, ht_hash_table*);

#endif
