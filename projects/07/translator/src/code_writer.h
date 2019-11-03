#ifndef _VM_ENCODER_H
#define _VM_ENCODER_H

#include <stdio.h>

FILE* new_parser(char*);
void set_filename(char*);
void write_arithmetic(char*);
void write_push_pop(char*);
void close();

#endif
