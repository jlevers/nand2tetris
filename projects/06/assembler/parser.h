/*
 * Header file for the parser for nand2tetris assembler.
 * @author Jesse Evers
 * @email jesse27999@gmail.com
 * @created 5/22/19
 */

#ifndef _PARSER_H
#define _PARSER_H

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
void process_file();
int has_more_commands(FILE*);
char* advance(FILE*);
command_t command_type();
char* symbol(command_t, const char*);
char* parse_dest(const char*);
char* parse_comp(const char*);
char* parse_jump(const char*);

#endif
