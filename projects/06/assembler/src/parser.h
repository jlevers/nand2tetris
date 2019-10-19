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
