#ifndef _VM_CODE_WRITER_H
#define _VM_CODE_WRITER_H

#include <stdio.h>
#include "parser.h"


// A struct that holds info about a VM memory segment
typedef struct vm_mem_seg {
    const char *vm_name;    // The name of the segment in the VM language
    const char *hack_name;  // The name of the segment's register(s) in Hack
    const int begin_addr;   // The RAM address of the beginning of the segment
    const int end_addr;     // The RAM address of the end of the segment
} vm_mem_seg;

// Return status types for vm_write_command
typedef enum vm_wc_status {
    WC_SUCCESS,
    WC_INVALID_CMD,
    WC_UNSUPPORTED_CMD
} vm_wc_status;


extern const char* FOUT_EXT;  // The file extension for the output file
extern const char DIR_SEP;    // The character used to separate directories in a file path

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


FILE* VM_Code_Writer(char*);
void vm_set_filename(char*);
vm_wc_status vm_write_command(char*, vm_command_t);
char *vm_translate_arithmetic(char*);
char *vm_translate_push(vm_mem_seg, int);
char *vm_translate_pop(vm_mem_seg, int);
void vm_code_writer_close();

#endif
