/*
 * Header file for the parser for nand2tetris assembler.
 * @author Jesse Evers
 * @email jesse27999@gmail.com
 * @created 5/22/19
 */

#include <stdio.h>
#include <stdlib.h>

typedef enum command_type {
  A_COMMAND = 0,
  C_COMMAND = 1,
  L_COMMAND = -1
} command_t;

FILE* init(const char*);
int process_file();
int has_more_commands();
void advance();
command_t command_type();
char* dest();
char* comp();
char* jump();
