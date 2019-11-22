#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "code_writer.h"
#include "parser.h"

#ifndef _VM_CODE_WRITER_VARS
#define _VM_CODE_WRITER_VARS

const char* FOUT_EXT = ".asm";
const char DIR_SEP = '/';

const vm_mem_seg SP = {NULL, "SP", 0, 0};
const vm_mem_seg LCL = {"local", "LCL", 1, 1};
const vm_mem_seg ARG = {"argument", "ARG", 2, 2};
const vm_mem_seg THIS = {"this", "THIS", 3, 3};
const vm_mem_seg THAT = {"that", "THAT", 4, 4};
const vm_mem_seg POINTER = {"pointer", NULL, 3, 4};
const vm_mem_seg TEMP = {"temp", "TEMP", 5, 12};
const vm_mem_seg GENERAL = {"general", NULL, 13, 15};
// This segment doesn't actually reference the entire memory, but saying that it does
// makes some math in vm_translate_push work nicely.
const vm_mem_seg CONSTANT = {"constant", NULL, 0, 32767};
const vm_mem_seg STATIC = {"static", NULL, 16, 255};
const vm_mem_seg STACK = {"stack", NULL, 256, 2047};
const vm_mem_seg HEAP = {"heap", NULL, 2048, 16483};
const vm_mem_seg MEMMAP_IO = {"io", NULL, 16384, 24575};
const vm_mem_seg SEG_INVALID = {NULL, NULL, -1, -1};

#endif

/* STATIC FUNCTIONS */

/**
 * Encodes the assembly commands needed to increment the stack pointer.
 * 
 * @return  char*  The assembly code to increment the stack pointer.
 */
static char *inc_sp() {
    return "@SP\n"
           "A=A+1\n";
}


/**
 * Encodes the assembly commands needed to decrement the stack pointer.
 * 
 * @return  char*  The assembly code to decrement the stack pointer.
 */
// static char *dec_sp() {
//     return "@SP\n"
//            "A=A-1";
// }


/**
 * Returns the vm_mem_seg corresponding to the text name of a memory segment.
 * 
 * @param  char*  segment  The segment for which to retrieve the corresponding vm_mem_seg
 * @return vm_mem_seg      The corresponding vm_mem_seg
 */
static vm_mem_seg seg_to_vm_memseg(char *segment) {
    if (!strcmp(segment, "local")) {
        return LCL;
    } else if (!strcmp(segment, "argument")) {
        return ARG;
    } else if (!strcmp(segment, "this")) {
        return THIS;
    } else if (!strcmp(segment, "that")) {
        return THAT;
    } else if (!strcmp(segment, "pointer")) {
        return POINTER;
    } else if (!strcmp(segment, "temp")) {
        return TEMP;
    } else if (!strcmp(segment, "general")) {
        return GENERAL;
    } else if (!strcmp(segment, "constant")) {
        return CONSTANT;
    } else if (!strcmp(segment, "static")) {
        return STATIC;
    } else if (!strcmp(segment, "stack")) {
        return STACK;
    } else if (!strcmp(segment, "heap")) {
        return HEAP;
    } else if (!strcmp(segment, "io")) {
        return MEMMAP_IO;
    } else {
        return SEG_INVALID;
    }
}


/**
 * Compares two strings using strcmp, but also allows either string given to be NULL.
 *
 * 
 * @param  char*  a  The first string to compare
 * @param  char*  b  The second string to compare
 * @return int       0 if the strings are equal, 1 if only one of them is NULL, and the value of
 *                   strcmp(a, b) otherwise
 */
static int vm_strcmp(const char *a, const char *b) {
    int diff;
    if (a == NULL && b == NULL) {
        diff = 0;
    } else if (a == NULL || b == NULL) {
        diff = 1;
    } else {
        diff = strcmp(a, b);
    }

    return diff;
}


/**
 * Compares two vm_mem_seg structs.
 * 
 * @param  vm_mem_seg  a  The first vm_mem_seg to compare
 * @param  vm_mem_seg  b  The second vm_mem_seg to compare
 * @return int            1 if the a and be are equivalent, 0 otherwise
 */
static int segcmp(vm_mem_seg a, vm_mem_seg b) {
    return !vm_strcmp(a.vm_name, b.vm_name) && !vm_strcmp(a.hack_name, b.hack_name)
        && a.begin_addr == b.begin_addr && a.end_addr == b.end_addr;
}


/**
 * Counts the number of digits in the given integer.
 * 
 * @param  int  num  The number whose number of digits is to be counted
 * @return int       The number of digits in |num|
 */
static int num_digits(const int num) {
    int copy = num;
    int count = 0;
    while (copy > 0) {
        count++;
        copy /= 10;
    }
    return count;
}


/**
 * Checks if the given path is a directory.
 * 
 * @param  char*  path  The path to check
 * @return int          1 if the path is a directory, 0 otherwise
 */
static int is_directory(const char *path) {
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISDIR(path_stat.st_mode);
}

/* END STATIC FUNCTIONS */



/**
 * Opens the output file to which to write the assembly code that reuslts from translating the input
 * file(s). If @input_path is a path to a file, the output file will be named the same thing as the
 * input file, but with the file extension FOUT_EXT instead of .vm. If @input_path is a path to a
 * folder, the output file will be named the same thing as the input folder, but with the file
 * extension FOUT_EXT added to the end. In both cases, the output file will be created in the folder
 * that contains the input file/folder.
 *
 * @param  char*  input_path  The path to the file or folder being translated to assembly
 * @return FILE*              A file pointer to the output file
 */
FILE *VM_Code_Writer(char *input_path) {
    // Check if input_path is a directory
    int is_dir = is_directory(input_path);

    // Make sure we ignore any trailing DIR_SEP (e.g., the last character of the path
    // "/with/trailing/slash/") if it exists
    int input_path_len = (int)strlen(input_path);
    if (input_path[input_path_len - 1] == DIR_SEP) {
        input_path_len--;
    }

    char *path_copy = calloc(input_path_len + 1, sizeof(char));
    strncpy(path_copy, input_path, input_path_len);
    path_copy[input_path_len] = '\0';

    int last_slash_idx = input_path_len - 1;
    while(last_slash_idx >= 0 && input_path[last_slash_idx] != DIR_SEP) { last_slash_idx--; }

    int outfile_name_len = input_path_len - (last_slash_idx + 1);

    int period_idx = -1;
    // Only need to worry about handling the input file's file extension if the input path was a
    // path to a file
    if (!is_dir) {
        for (int i = strlen(path_copy) - 1; i >= 0; i--) {
            if (path_copy[i] == '.') {
                period_idx = i;
                break;
            }
        }

        // Remove the length of '.' + file extension from outfile_name_len, if the input file has
        // a file extension
        if (period_idx > -1) {
            outfile_name_len -= input_path_len - (period_idx + 1);
        }
    }

    // If there was no period in the input path, set period_idx to the length of the input path
    if (period_idx == -1) period_idx = input_path_len;

    // Get just the actual filename to write to -- e.g., outfile.asm
    char *outfile_name = calloc(outfile_name_len + (int)strlen(FOUT_EXT) + 1, sizeof(char));
    strncpy(outfile_name, path_copy + last_slash_idx + 1, period_idx - (last_slash_idx + 1));
    outfile_name[outfile_name_len] = '\0';
    strcat(outfile_name, FOUT_EXT);

    // Combine the path to the output file with the actual name of the output file. E.g.,
    // ./example/path/to/outfile.asm
    int full_output_path_len = (int)strlen(outfile_name) + (last_slash_idx + 1);
    char *output_path = calloc(full_output_path_len + 1, sizeof(char));
    strncpy(output_path, path_copy, last_slash_idx + 1);
    output_path[last_slash_idx + 1] = '\0';
    strcat(output_path, outfile_name);

    FILE *out = fopen(output_path, "a");

    free(path_copy);
    free(outfile_name);
    free(output_path);

    if (!out) {
        perror("[ERR] Failed to open VM output file");
        return NULL;
    }

    return out;
}


/**
 * Writes a VM command in .hack ASM format.
 *
 * @param  char*         command       The VM command to translate
 * @param  vm_command_t  command_type  The command type of the VM command
 * @return vm_wc_status                The status of the function
 */
vm_wc_status vm_write_command(char *command, vm_command_t command_type) {
    vm_wc_status status = WC_SUCCESS;

    if (command_type == C_PUSH || command_type == C_POP) {
        char *segment = vm_arg1(command);
        int index = vm_arg2(command);
        if (segment != NULL && strlen(segment) && index > -1) {
            vm_mem_seg seg = seg_to_vm_memseg(segment);
            if (command_type == C_PUSH) {
                vm_translate_push(seg, index);
            } else if (command_type == C_POP) {
                vm_translate_pop(seg, index);
            }
            free(segment);
            segment = NULL;
        } else {
            const char *err = "[ERR] Result of calling vm_arg1() on command \"%s\" was NULL. "
                              "Cannot translate command\n";
            printf(err, command);
            status = WC_INVALID_CMD;
        }
    } else if (command_type == C_ARITHMETIC) {
        vm_translate_arithmetic(command);
    } else if (command_type == C_INVALID) {
        printf("[ERR] Invalid command %s\n", command);
        status = WC_INVALID_CMD;
    } else {
        printf("[ERR] Command type %d is not currently supported.\n", command_type);
        status = WC_UNSUPPORTED_CMD;
    }

    return status;
}


/**
 * Encodes arithmetic VM operations as Hack assembly code.
 *
 * @param  char*  command  The arithmetic command to encode
 * @return char*           The encoded version of |command|
 */
char *vm_translate_arithmetic(char *command) {
    // Removes any extra whitespace, makes sure it's valid, etc.
    char *cmd = vm_arg1(command);

    if (!strcmp(cmd, "add")) {

    } else if (!strcmp(cmd, "sub")) {

    } else if (!strcmp(cmd, "neg")) {

    } else if (!strcmp(cmd, "eq")) {

    } else if (!strcmp(cmd, "gt")) {

    } else if (!strcmp(cmd, "lt")) {

    } else if (!strcmp(cmd, "and")) {

    } else if (!strcmp(cmd, "or")) {

    } else if (!strcmp(cmd, "not")) {

    }

    return NULL;
}


/**
 * Translates a VM push command into Hack assembly code.
 * 
 * @param  vm_mem_seg  segment  The segment that data is being pushed from
 * @param  int         index    The index in the segment to select data from
 * @return char*                The translated assembly code
 */
char *vm_translate_push(vm_mem_seg segment, int index) {
    char *push_encoded = NULL;
    // The value to push onto the stack is the value starting at the index-th value in 
    // the given segment
    int mem_addr = index + segment.begin_addr;

    if (segcmp(segment, SEG_INVALID)) {
        printf("[ERR] Invalid memory segment %s\n", segment.vm_name);
    // Validate the memory address
    } else if (mem_addr < 0 || mem_addr > segment.end_addr) {
        printf("[ERR] Invalid memory address %d in segment %s\n", mem_addr, segment.vm_name);
    } else if (segcmp(segment, CONSTANT)) {
        // To push a constant value onto the stack, we have to increment the stack pointer,
        // store that value in D, and then put that value in the memory address pointed to by
        // the stack pointer.

        char *increment_sp = inc_sp();
        char *push = calloc(11 + num_digits(mem_addr) + 1, sizeof(char));

        sprintf(push,
                "D=%d\n"
                "@SP\n"
                "M=D\n",
                mem_addr);

        // Construct the command
        int encoded_cmd_len = strlen(increment_sp) + strlen(push) + 1;
        push_encoded = calloc(encoded_cmd_len, sizeof(char));
        push_encoded[encoded_cmd_len] = '\0';
        strcat(push_encoded, increment_sp);
        strcat(push_encoded, push);
    } else {
        printf("[ERR] Pushing from the segment %s is not yet supported\n", segment.vm_name);
        // sprintf(push,
        //         "@%d\n"                     // Get the value from the heap
        //         "D=M\n"
        //         "@SP\n"                     // Push it onto the stack
        //         "M=D\n",
        //         mem_addr);
    }

    return push_encoded;
}


char *vm_translate_pop(vm_mem_seg segment, int index) {
    printf("%s %d\n", segment.vm_name, index);
    return NULL;
}
