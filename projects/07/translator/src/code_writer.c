#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "code_writer.h"
#include "../../../lib/hash_table.h"
#include "parser.h"
#include "util.h"


/*
 * NOTES
 * -----
 * For arithmetic operations, the address to jump to after completing the operation is stored in @R13.
 * For true/false operations, the address to jump to after completing the operation is stored in @R14.
 * All internal labels begin with a double underscore: __LABEL_NAME
 */


#ifndef _VM_CODE_WRITER_VARS
#define _VM_CODE_WRITER_VARS

const char *FOUT_EXT = ".asm";
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

const char *INIT_SP =
    "// Initialize stack pointer\n"
    "@256\n"
    "D=A\n"
    "@SP\n"
    "M=D";

const char *INF_LOOP =
    "(__INFINITE_LOOP)\n"
    "@__INFINITE_LOOP\n"
    "0;JMP\n";

const char *TF_FUNC =
    "(__TRUE)\n"
    "@SP\n"
    "A=M-1\n"
    "M=-1\n"
    "@R14\n"
    "A=M\n"
    "0;JMP\n"
    "(__FALSE)\n"
    "@SP\n"
    "A=M-1\n"
    "M=0\n"
    "@R14\n"
    "A=M\n"
    "0;JMP";

const char *ARITH_OP_END =
    "(__END_ARITH_OP)\n"
    "@R13\n"
    "A=M\n"
    "0;JMP\n";

/*
 * All the following ARITH_<OPTYPE>_BASE_CMD constants are missing their function labels, which are generated
 * in gen_arith_cmd() when the command is written to the end of the assembly file. Each base command contains a single
 * format specifier in the place of a specific arithmetic operator, which will be filled in by gen_arith_cmd().
 */
const char *ARITH_ADDSUB_BASE_CMD =
    "@SP\n"
    "A=M-1\n"
    "D=M\n"
    "@SP\n"
    "M=M-1\n"
    "A=M-1\n"
    "D=M%sD\n"
    "M=D\n"
    "@__END_ARITH_OP\n"
    "0;JMP\n";

const char *ARITH_CMP_BASE_CMD =
    "@SP\n"
    "A=M-1\n"
    "D=M\n"
    "@SP\n"
    "M=M-1\n"
    "A=M-1\n"
    "M=M-D\n"
    "@__END_ARITH_OP\n"
    "D=A\n"
    "@R14\n"
    "M=D\n"
    "@SP\n"
    "A=M-1\n"
    "D=M\n"
    "@__TRUE\n"
    "D;J%s\n"
    "@__FALSE\n"
    "0;JMP\n";

const char *ARITH_BOOL_BASE_CMD =
    "@SP\n"
    "A=M-1\n"
    "D=M\n"
    "@SP\n"
    "M=M-1\n"
    "A=M-1\n"
    "M=M%sD\n"
    "@__END_ARITH_OP\n"
    "0;JMP\n";

const char *ARITH_UNARY_BASE_CMD =
    "@SP\n"
    "A=M-1\n"
    "M=%sM\n"
    "@__END_ARITH_OP\n"
    "0;JMP\n";

const int NUM_ARITH_OPS = 9;
const char *VM_OPS[] = {"add", "sub", "neg", "eq", "gt", "lt", "and", "or", "not", NULL};
const char *ASM_OPS[] = {"+", "-", "-", "EQ", "GT", "LT", "&", "|", "!", NULL};


#endif


/* STATIC VARIABLES */

static int num_arith_calls = 0;

/* END STATIC VARIABLES */


/* STATIC FUNCTIONS */

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
 * Compares two vm_mem_seg structs.
 * 
 * @param a the first vm_mem_seg to compare
 * @param b the second vm_mem_seg to compare
 * @return  1 if the a and b are equivalent, 0 otherwise
 */
static int segcmp(vm_mem_seg a, vm_mem_seg b) {
    return !vm_strcmp(a.vm_name, b.vm_name) && !vm_strcmp(a.hack_name, b.hack_name)
        && a.begin_addr == b.begin_addr && a.end_addr == b.end_addr;
}


/**
 * Generates the internal label used in the final assembly file for the given VM operation.
 * 
 * @param op the operation for which to generate an internal label
 * @return   the internal label for the given VM operation (name only, no symbols like '@' or '()' added)
 */
static char *get_internal_op_label(const char *op) {
    const char *internal_op_base = "__%s_OP";
    int label_len = strlen(internal_op_base) - strlen("%s") + strlen(op);
    char *op_uppercase = calloc(strlen(op) + 1, sizeof(char));
    toupper_str(op_uppercase, op);

    char *label = calloc(label_len + 1, sizeof(char));
    snprintf(label, label_len + 1, internal_op_base, op_uppercase);
    free(op_uppercase);

    return label;
}


/**
 * Generates the assembly code for a VM arithmetic operator.
 *
 * @param base_cmd the base command for the category of command being generated (add/sub, comparison, etc)
 * @param op       the VM operation to generate assembly code for
 * @param op_map   a hashmap mapping VM operations to assembly operators
 * @return         the translated assembly code
 */
static char *gen_arith_cmd(const char *base_cmd, const char *op, ht_hash_table *op_map) {
    char *encoded_cmd = NULL;
    char *asm_op = ht_search(op_map, op);

    if (asm_op == NULL) {
        printf("[ERR] Invalid VM operation or invalid op_map given to gen_arith_cmd\n");
    } else {
        char *op_label = get_internal_op_label(op);
        const char *label_fmt_str = "(%s)\n";
        int label_fmt_str_len = strlen(label_fmt_str);
        int base_cmd_len = strlen(base_cmd);
        int final_base_cmd_len = base_cmd_len + label_fmt_str_len;

        char *base_cmd_w_label = calloc(final_base_cmd_len + 1, sizeof(char));
        base_cmd_w_label[0] = '\0';
        strncat(base_cmd_w_label, label_fmt_str, label_fmt_str_len);
        strncat(base_cmd_w_label, base_cmd, base_cmd_len);

        // The length of "%s" times 2: once in label_fmt_str and once in for the assembly operation in base_cmd
        int fmt_specifiers_len = 4;
        int cmd_len = final_base_cmd_len - fmt_specifiers_len + strlen(op_label) + strlen(asm_op);

        encoded_cmd = calloc(cmd_len + 1, sizeof(char));
        snprintf(encoded_cmd, cmd_len + 1, base_cmd_w_label, op_label, asm_op);

        free(op_label);
        free(base_cmd_w_label);
        free(asm_op);
    }

    return encoded_cmd;
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
 * @param input_path the path to the file or folder being translated to assembly
 * @return           a file pointer to the output file
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

    FILE *out = fopen(output_path, "w");

    free(path_copy);
    free(outfile_name);

    fprintf(out, "// Generated by Nand2Tetris VM translator written by Jesse Evers\n");
    fprintf(out, "// File: %s\n\n", output_path);
    fprintf(out, "%s\n\n", INIT_SP);
    fprintf(out, "// Begin user-defined program\n");

    if (!out) {
        perror("[ERR] Failed to open VM output file");
        return NULL;
    }

    free(output_path);

    return out;
}


/**
 * Writes a VM command in .hack ASM format.
 *
 * @param command      the VM command to translate
 * @param command_type the command type of the VM command
 * @param to_write     the file to write the translated command to
 * @return             the status of the function
 */
vm_wc_status vm_write_command(char *command, vm_command_t command_type, FILE *to_write) {
    vm_wc_status status = WC_SUCCESS;
    char *translated = NULL;

    if (command_type == C_PUSH || command_type == C_POP) {
        char *segment = vm_arg1(command);
        int index = vm_arg2(command);
        if (segment != NULL && strlen(segment) && index > -1) {
            vm_mem_seg seg = seg_to_vm_memseg(segment);
            if (command_type == C_PUSH) {
                translated = vm_translate_push(seg, index);
            } else if (command_type == C_POP) {
                translated = vm_translate_pop(seg, index);
            }
        } else {
            const char *err = "[ERR] Result of calling vm_arg1() on command \"%s\" was NULL. "
                              "Cannot translate command\n";
            printf(err, command);
            status = WC_INVALID_CMD;
        }
        free(segment);
        segment = NULL;
    } else if (command_type == C_ARITHMETIC) {
        translated = vm_translate_arithmetic(command);
    } else if (command_type == C_INVALID) {
        printf("[ERR] Invalid command %s\n", command);
        status = WC_INVALID_CMD;
    } else {
        printf("[ERR] Command type %d is not currently supported.\n", command_type);
        status = WC_UNSUPPORTED_CMD;
    }

    if (status == WC_SUCCESS && translated != NULL) {
        fprintf(to_write, "%s", translated);
    } else {
        printf("[ERR] Not writing translation of command \"%s\" to file due to vm_wc_status code"
               " %d\n", command, status);
    }

    free(translated);
    translated = NULL;
    return status;
}


/**
 * Encodes arithmetic VM operations as Hack assembly code.
 *
 * @param command the arithmetic command to encode
 * @return        the encoded version of |command|
 */
char *vm_translate_arithmetic(char *command) {
    // Removes any extra whitespace, makes sure it's valid, etc.
    char *cmd = vm_arg1(command);

    const char *goto_str =
        "@POST_ARITH_CALL_%d\n"
        "D=A\n"
        "@R13\n"
        "M=D\n"
        "@%s\n"
        "0;JMP\n"
        "(POST_ARITH_CALL_%d)\n";
    char *goto_label = get_internal_op_label(cmd);

    int goto_str_len = strlen(goto_str);
    int fmt_specifiers_len = 6;  // Length of "%s" + 2 * length of "%d"
    int total_len = goto_str_len - fmt_specifiers_len + strlen(goto_label) + 2 * num_digits(num_arith_calls);

    char *encoded_goto = calloc(total_len + 1, sizeof(char));
    snprintf(encoded_goto, total_len + 1, goto_str, num_arith_calls, goto_label, num_arith_calls);

    free(goto_label);
    free(cmd);

    num_arith_calls++;

    return encoded_goto;
}


/**
 * Translates a VM push command into Hack assembly code.
 *
 * @param segment the segment that data is being pushed from
 * @param index   the index in the segment to select data from
 * @return        the translated assembly code
 */
char *vm_translate_push(vm_mem_seg segment, int index) {
    char *push_encoded = NULL;
    // The value to push onto the stack is the value starting at the index-th value in 
    // the given segment
    int mem_addr = index + segment.begin_addr;

    if (segcmp(segment, SEG_INVALID)) {
        printf("[ERR] Invalid memory segment %s\n", segment.vm_name == NULL ? "(unnamed segment)" : segment.vm_name);
    // Validate the memory address
    } else if (mem_addr < 0 || mem_addr > segment.end_addr) {
        printf("[ERR] Invalid memory address %d in segment %s\n", mem_addr, segment.vm_name);
    } else if (segcmp(segment, CONSTANT)) {
        // To push a constant value onto the stack, we have to increment the stack pointer,
        // store that value in D, and then put that value in the memory address pointed to by
        // the stack pointer.
        const char *push_constant =
            "@SP\n"
            "M=M+1\n"
            "@%d\n"
            "D=A\n"
            "@SP\n"
            "A=M-1\n"
            "M=D\n";
        int format_specifiers_len = 2;  // Length of "%d"
        int encoded_len = strlen(push_constant) - format_specifiers_len + num_digits(mem_addr);

        push_encoded = calloc(encoded_len + 1, sizeof(char));
        snprintf(push_encoded, encoded_len + 1, push_constant, mem_addr);
    } else {
        printf("[ERR] Pushing from the segment %s is not yet supported\n", segment.vm_name);
    }

    return push_encoded;
}


char *vm_translate_pop(vm_mem_seg segment, int index) {
    printf("%s %d\n", segment.vm_name, index);
    return NULL;
}


/**
 * Writes system-generated code to the output file, and closes it.
 *
 * @param cw the file to write all assembly code to
 */
void vm_code_writer_close(FILE *cw) {
    fprintf(cw, "// End user-defined program\n\n");
    fprintf(cw, "// This terminates the program by sending it into an infinite loop\n");
    fprintf(cw, "%s\n", INF_LOOP);
    /*
     * All system-level assembly routines should go after this line. Any assembly routines placed before this
     * may end up being run in the middle of the user's program.
     */

    // A map of VM operations (add, sub, etc) and the assembly commands associated with them
    ht_hash_table *vm_op_to_asm = ht_new(NUM_ARITH_OPS);
    ht_insert_all(vm_op_to_asm, NUM_ARITH_OPS, VM_OPS, ASM_OPS);
    
    // Arithmetic operations (this could be made DRYer, but I think it's more clear when written out)
    fprintf(cw, "%s\n", gen_arith_cmd(ARITH_ADDSUB_BASE_CMD, "add", vm_op_to_asm));
    fprintf(cw, "%s\n", gen_arith_cmd(ARITH_ADDSUB_BASE_CMD, "sub", vm_op_to_asm));
    fprintf(cw, "%s\n", gen_arith_cmd(ARITH_CMP_BASE_CMD, "eq", vm_op_to_asm));
    fprintf(cw, "%s\n", gen_arith_cmd(ARITH_CMP_BASE_CMD, "gt", vm_op_to_asm));
    fprintf(cw, "%s\n", gen_arith_cmd(ARITH_CMP_BASE_CMD, "lt", vm_op_to_asm));
    fprintf(cw, "%s\n", gen_arith_cmd(ARITH_BOOL_BASE_CMD, "and", vm_op_to_asm));
    fprintf(cw, "%s\n", gen_arith_cmd(ARITH_BOOL_BASE_CMD, "or", vm_op_to_asm));
    fprintf(cw, "%s\n", gen_arith_cmd(ARITH_UNARY_BASE_CMD, "neg", vm_op_to_asm));
    fprintf(cw, "%s\n", gen_arith_cmd(ARITH_UNARY_BASE_CMD, "not", vm_op_to_asm));

    ht_delete(vm_op_to_asm);

    // Assists jump back to primary program flow after arithmetic operations
    fprintf(cw, "%s\n", ARITH_OP_END);

    // Enables true/false operations
    fprintf(cw, "%s\n", TF_FUNC);
}