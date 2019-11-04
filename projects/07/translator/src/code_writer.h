#ifndef _VM_CODE_WRITER_H
#define _VM_CODE_WRITER_H

#include <stdio.h>


extern const char* FOUT_EXT;  // The file extension for the output file
extern const char DIR_SEP;    // The character used to separate directories in a file path 

FILE* VM_Code_Writer(char*);
void vm_set_filename(char*);
void vm_write_arithmetic(char*);
void vm_write_push_pop(char*);
void vm_code_writer_close();

#endif
