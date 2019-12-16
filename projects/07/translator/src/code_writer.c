#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asm_constants.h"
#include "code_writer.h"
#include "parser.h"
#include "util.h"
#include "vm_constants.h"
#include "../../../lib/hash_table.h"


#ifndef _VM_CODE_WRITER_VARS
#define _VM_CODE_WRITER_VARS

const char *FOUT_EXT = ".asm";

// See code_writer.h for details on how vm_mem_seg is used
const vm_mem_seg SP = {NULL, "SP", 0, 0};
const vm_mem_seg LCL = {"local", "LCL", 1, -1};
const vm_mem_seg ARG = {"argument", "ARG", 2, -1};
const vm_mem_seg THIS = {"this", "THIS", 3, -1};
const vm_mem_seg THAT = {"that", "THAT", 4, -1};
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
static char *get_internal_op_label(char *op) {
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
static char *gen_arith_cmd(const char *base_cmd, char *op, ht_hash_table *op_map) {
    char *encoded_cmd = NULL;
    char *asm_op = ht_search(op_map, op);

    if (asm_op == NULL) {
        printf("[ERR] Invalid VM operation or invalid op_map given to gen_arith_cmd\n");
    } else {
        char *op_label = get_internal_op_label(op);
        char *label_fmt_str = "(%s)\n";
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
 * Populates the code_writer struct to use to write the assembly code that reuslts from translating the input
 * file(s).
 * 
 * If @input_path is a path to a file, the output file will be named the same thing as the
 * input file, but ending with FOUT_EXT instead of .vm. If @input_path is a path to a
 * folder, the output file will have the same name as the input folder, with the file extension FOUT_EXT
 * added. In both cases, the output file will be created in the folder that contains the input file/folder.
 * 
 * @cw will initially have its in_name field set to NULL.
 *
 * @param input_path the path to the file or folder being translated to assembly
 * @param cw         the code_writer struct to use for translation
 */
void VM_Code_Writer(char *input_path, code_writer *cw) {
    path_parts *input_parts = calloc(1, sizeof(path_parts));
    input_parts->basename = calloc(strlen(input_path) + 1, sizeof(char));
    input_parts->dirname = calloc(strlen(input_path) + 1, sizeof(char));
    path_parts_split(input_parts, input_path);

    char *outfile_base_name = remove_fext(input_parts->basename);
    int outfile_base_name_len = strlen(outfile_base_name);

    // Get just the actual filename to write to -- e.g., outfile.asm
    char *outfile_name = calloc(outfile_base_name_len + strlen(FOUT_EXT) + 1, sizeof(char));
    strncpy(outfile_name, outfile_base_name, outfile_base_name_len);
    outfile_name[outfile_base_name_len] = '\0';
    strncat(outfile_name, FOUT_EXT, strlen(FOUT_EXT));

    // Combine the path to the output file with the actual name of the output file. E.g.,
    // ./example/path/to/outfile.asm
    int dirname_len = strlen(input_parts->dirname);
    int outfile_name_len = strlen(outfile_name);
    int output_path_len = outfile_name_len + dirname_len;
    char *output_path = calloc(output_path_len + 1, sizeof(char));
    output_path[output_path_len] = '\0';
    strncpy(output_path, input_parts->dirname, dirname_len);
    strncat(output_path, outfile_name, outfile_name_len);

    FILE *out = fopen(output_path, "w");

    if (!out) {
        perror("[ERR] Failed to open VM output file");
    } else {
        cw->out = out;
    }

    fprintf(out, "// Generated by Nand2Tetris VM translator written by Jesse Evers\n");
    fprintf(out, "// File: %s\n\n", output_path);
    fprintf(out, "%s\n\n", INIT_SP);
    fprintf(out, "// Begin user-defined program\n");

    free(outfile_base_name);
    free(outfile_name);
    free(output_path);
    del_path_parts(&input_parts);
}


/**
 * Writes a VM command in .hack ASM format.
 *
 * @param command      the VM command to translate
 * @param command_type the command type of the VM command
 * @param cw           the code_writer to use to write the translated command
 * @return             the status of the function
 */
vm_wc_status vm_write_command(char *command, vm_command_t command_type, code_writer *cw) {
    vm_wc_status status = WC_SUCCESS;
    char *translated = NULL;

    if (command_type == C_PUSH || command_type == C_POP) {
        char *segment = vm_arg1(command);
        int index = vm_arg2(command);
        if (segment != NULL && strlen(segment) && index > -1) {
            vm_mem_seg seg = seg_to_vm_memseg(segment);
            translated = vm_translate_push_pop(seg, index, command_type, cw->in_name);
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
        fprintf(cw->out, "%s", translated);
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
 * Translates a VM push or pop command into Hack assembly code.
 *
 * @param segment  the segment that data is being pushed from/popped to
 * @param index    the index in the segment to select data from
 * @param cmd_type the type of command to translate (push or pop)
 * @param in_fname the name (without a file extension) of the .vm file being translated
 * @return         the translated assembly code
 */
char *vm_translate_push_pop(vm_mem_seg segment, int index, vm_command_t cmd_type, char *in_fname) {
    char *push_encoded = NULL;
    const char *base_asm_cmds;
    // The value to push onto the stack is the value starting at the index-th value in 
    // the given segment
    int mem_addr = index + segment.begin_addr;
    int fmt_specifiers_len, translated_len;

    if (segcmp(segment, SEG_INVALID)) {
        printf("[ERR] Invalid memory segment %s\n", segment.vm_name == NULL ? "(unnamed segment)" : segment.vm_name);
    // Validate the memory address
    } else if (mem_addr < 0 || (segment.end_addr > -1 && mem_addr > segment.end_addr)) {
        printf("[ERR] Invalid memory address %d in segment %s\n", mem_addr, segment.vm_name);
    } else if (segcmp(segment, CONSTANT)) {
        /*
         * To push a constant value onto the stack, we have to increment the stack pointer,
         * store that value in D, and then put that value in the memory address pointed to by
         * the stack pointer.
         * 
         * Since the constant "segment" is a virtual segment with no RAM associated with it,
         * the "pop" command cannot be used with it.
         */
        base_asm_cmds = PUSH_CONSTANT_SEG;
        fmt_specifiers_len = 2;
        translated_len = strlen(base_asm_cmds) - fmt_specifiers_len + num_digits(mem_addr);
        push_encoded = calloc(translated_len + 1, sizeof(char));
        snprintf(push_encoded, translated_len + 1, base_asm_cmds, mem_addr);
    } else if (segcmp(segment, LCL) || segcmp(segment, ARG) || segcmp(segment, THIS) || segcmp(segment, THAT)) {
        base_asm_cmds = (cmd_type == C_PUSH ? PUSH_VIRTUAL_SEG : POP_VIRTUAL_SEG);
        fmt_specifiers_len = 4;
        translated_len = strlen(base_asm_cmds) - fmt_specifiers_len + strlen(segment.hack_name) + num_digits(index);
        push_encoded = calloc(translated_len + 1, sizeof(char));
        snprintf(push_encoded, translated_len + 1, base_asm_cmds, segment.hack_name, index);
    } else if (segcmp(segment, POINTER) || segcmp(segment, TEMP)) {
        base_asm_cmds = (cmd_type == C_PUSH ? PUSH_POINTER_SEG : POP_POINTER_SEG);
        fmt_specifiers_len = 2;
        translated_len = strlen(base_asm_cmds) - fmt_specifiers_len + num_digits(mem_addr);
        push_encoded = calloc(translated_len + 1, sizeof(char));
        snprintf(push_encoded, translated_len + 1, base_asm_cmds, mem_addr);
    } else if (segcmp(segment, STATIC)) {
        base_asm_cmds = (cmd_type == C_PUSH ? PUSH_STATIC_SEG : POP_STATIC_SEG);
        fmt_specifiers_len = 4;
        translated_len = strlen(base_asm_cmds) - fmt_specifiers_len + strlen(in_fname) + num_digits(index);
        push_encoded = calloc(translated_len + 1, sizeof(char));
        snprintf(push_encoded, translated_len + 1, base_asm_cmds, in_fname, index);
    } else {
        printf("[ERR] Pushing from the segment %s is not supported\n", segment.vm_name);
    }

    return push_encoded;
}


/**
 * Writes system-generated code to the output file, and closes it.
 *
 * @param cw the code_writer used to write all translated assembly code
 */
void vm_code_writer_close(code_writer *cw) {
    FILE *out = cw->out;

    fprintf(out, "// End user-defined program\n\n");
    fprintf(out, "// This terminates the program by sending it into an infinite loop\n");
    fprintf(out, "%s\n", INF_LOOP);
    /*
     * All system-level assembly routines should go after this line. Any assembly routines placed before this
     * may end up being run in the middle of the user's program.
     */

    // A map of VM operations (add, sub, etc) and the assembly commands associated with them
    ht_hash_table *vm_op_to_asm = ht_new(NUM_ARITH_OPS);
    ht_insert_all(vm_op_to_asm, NUM_ARITH_OPS, ARITHMETIC_OPS, ASM_OPS);
    
    // Arithmetic operations (this could be made DRYer, but I think it's more clear when written out)
    char *add_op = gen_arith_cmd(ARITH_ADDSUB_BASE_CMD, "add", vm_op_to_asm);
    char *sub_op = gen_arith_cmd(ARITH_ADDSUB_BASE_CMD, "sub", vm_op_to_asm);
    char *eq_op = gen_arith_cmd(ARITH_CMP_BASE_CMD, "eq", vm_op_to_asm);
    char *gt_op = gen_arith_cmd(ARITH_CMP_BASE_CMD, "gt", vm_op_to_asm);
    char *lt_op = gen_arith_cmd(ARITH_CMP_BASE_CMD, "lt", vm_op_to_asm);
    char *and_op = gen_arith_cmd(ARITH_BOOL_BASE_CMD, "and", vm_op_to_asm);
    char *or_op = gen_arith_cmd(ARITH_BOOL_BASE_CMD, "or", vm_op_to_asm);
    char *neg_op = gen_arith_cmd(ARITH_UNARY_BASE_CMD, "neg", vm_op_to_asm);
    char *not_op = gen_arith_cmd(ARITH_UNARY_BASE_CMD, "not", vm_op_to_asm);
    fprintf(out, "%s\n", add_op);
    fprintf(out, "%s\n", sub_op);
    fprintf(out, "%s\n", eq_op);
    fprintf(out, "%s\n", gt_op);
    fprintf(out, "%s\n", lt_op);
    fprintf(out, "%s\n", and_op);
    fprintf(out, "%s\n", or_op);
    fprintf(out, "%s\n", neg_op);
    fprintf(out, "%s\n", not_op);
    reinit_char(&add_op);
    reinit_char(&sub_op);
    reinit_char(&eq_op);
    reinit_char(&gt_op);
    reinit_char(&lt_op);
    reinit_char(&and_op);
    reinit_char(&or_op);
    reinit_char(&neg_op);
    reinit_char(&not_op);
    

    ht_delete(vm_op_to_asm);

    // Assists jump back to primary program flow after arithmetic operations
    fprintf(out, "%s\n", ARITH_OP_END);

    // Enables true/false operations
    fprintf(out, "%s\n", TF_FUNC);

    if (out != NULL) {
        fclose(out);
        cw->out = NULL;
    }
    if (cw->in_name != NULL) {
        reinit_char(&(cw->in_name));
    }
    if (cw != NULL) {
        free(cw);
        cw = NULL;
    }
}