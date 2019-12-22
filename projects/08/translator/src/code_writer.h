#ifndef _VM_CODE_WRITER_H
#define _VM_CODE_WRITER_H

#include <stdio.h>
#include <stdlib.h>

#include "parser.h"


// Stores the file that the code writer writes to, and the name of the .vm file currently being translated
typedef struct code_writer {
    // The file to write to
    FILE *out;
    char *in_name;
} code_writer;

// A struct that holds info about a VM memory segment
typedef struct vm_mem_seg {
    // The name of the segment in the VM language
    char *vm_name;
    // The name of the segment's register(s) in Hack
    char *hack_name;
    // The RAM address of the beginning of the segment
    int begin_addr;
    // The RAM address of the end of the segment (must be -1 if this is a segment which is accessed via a pointer
    // to the beginning of an object elsewhere in memory, and therefore has no hard limit on how large of a segment
    // it can be)
    int end_addr;
} vm_mem_seg;

// Return status types for vm_write_command
typedef enum vm_wc_status {
    WC_SUCCESS,
    WC_INVALID_CMD,
    WC_UNSUPPORTED_CMD,
    WC_WRITE_FILE_ERR
} vm_wc_status;


extern const char* FOUT_EXT;  // The file extension for the output file


// Predefined memory segments
extern const vm_mem_seg SP;
extern const vm_mem_seg LCL;
extern const vm_mem_seg ARG;
extern const vm_mem_seg THIS;
extern const vm_mem_seg THAT;
extern const vm_mem_seg POINTER;
extern const vm_mem_seg TEMP;
extern const vm_mem_seg GENERAL;
extern const vm_mem_seg CONSTANT;
extern const vm_mem_seg STATIC;
extern const vm_mem_seg STACK;
extern const vm_mem_seg HEAP;
extern const vm_mem_seg MEMMAP_IO;
extern const vm_mem_seg SEG_INVALID;


void VM_Code_Writer(char*, code_writer*);
void vm_set_filename(char*);
vm_wc_status vm_write_command(char*, vm_command_t, code_writer*);
char *vm_translate_arithmetic(char*);
char *vm_translate_push_pop(vm_mem_seg, int, vm_command_t, char*);
void vm_code_writer_close(code_writer*);

#endif /* _VM_CODE_WRITER_H */
