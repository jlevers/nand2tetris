#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asm_constants.h"
#include "code_writer.h"
#include "parser.h"
#include "util.h"
#include "vm_constants.h"
#include "../../../lib/hash_table.h"


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


/* STATIC VARIABLES */

static int num_arith_calls = 0;
static int num_func_definitions = 0;

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

        fmt_str *final_base_cmd = fmt_str_new(NULL, 4);
        final_base_cmd->str = calloc(final_base_cmd_len + 1, sizeof(char));
        final_base_cmd->str[0] = '\0';
        strncat(final_base_cmd->str, label_fmt_str, label_fmt_str_len);
        strncat(final_base_cmd->str, base_cmd, base_cmd_len);

        int cmd_len = fmt_str_len(final_base_cmd) + strlen(op_label) + strlen(asm_op);

        encoded_cmd = calloc(cmd_len + 1, sizeof(char));
        snprintf(encoded_cmd, cmd_len + 1, final_base_cmd->str, op_label, asm_op);

        fmt_str_delete(&final_base_cmd);
        free(op_label);
        free(asm_op);
    }

    return encoded_cmd;
}


/**
 * Checks that a VM function or label contains only allowed characters.
 *
 * @param ident    the function/label to check
 * @param is_label 1 if @ident is a label name, 0 otherwise
 * @return         1 if @ident is valid, 0 otherwise
 */
static int valid_identifier(char *ident, int is_label) {
    char ch;
    int all_valid = 1;

    for (int i = 0; i < (int)strlen(ident); i++) {
        ch = ident[i];
        int valid = 0;
        for (int j = 0; j < (int)(sizeof(LABEL_CHAR_RANGES) / sizeof(LABEL_CHAR_RANGES[0])); j++) {
            if (ch >= LABEL_CHAR_RANGES[j][0] && ch <= LABEL_CHAR_RANGES[j][1]) {
                valid = 1;
            }
        }

        // Don't allow the FUNCTION_SEPARATOR character in label names
        if (is_label && ch == FUNCTION_SEPARATOR) {
            valid = 0;
        }

        if (!valid) {
            all_valid = 0;
        }
    }

    return all_valid;
}


/**
 * Returns a filled-in version of a fmt_str, given a list of variables to substitute in, and their total string length.
 * Essentially a wrapper around vsnprintf().
 *
 * @param fs      the fmt_str struct to fill in
 * @param sub_len the total length of all variables being substituted into @fs
 * @param ...     a list of variables to substitute into @fs
 * @return        the completed version of @fs, with all format specifiers replaced with the substitute variables
 */
static char *fmt_str_printf(const fmt_str *fs, int sub_len, ...) {
    int final_str_len = fmt_str_len(fs) + sub_len;
    char *final_str = calloc(final_str_len + 1, sizeof(char));
    va_list args;
    va_start(args, sub_len);
    vsnprintf(final_str, final_str_len + 1, fs->str, args);
    return final_str;
}


/**
 * Generates the Hack assembly code for label-related VM commands
 *
 * @param fs    the format string to be used to generate the Hack commands
 * @param func  the function containing the label
 * @param label the label name itself
 * @return      the Hack code to define or go to a VM label
 */
static char *gen_label_cmd(fmt_str fs, char *func, char *label) {
    char *cmd = NULL;

    char *internal_func = func != NULL ? strdup(func) : strdup(DEFAULT_FUNC_NAME);

    int valid = valid_identifier(internal_func, 0) && valid_identifier(label, 1);

    if (valid) {
        cmd = fmt_str_printf(&fs, strlen(internal_func) + strlen(label) + 1, internal_func, LABEL_SEPARATOR, label);
    } else {
        printf("[ERR] Invalid label %s defined or referenced in %s\n", label, internal_func);
    }

    reinit_str(&internal_func);

    return cmd;
}

/* END STATIC FUNCTIONS */


/**
 * Produces the code_writer struct to use to write the assembly code that results from translating the input
 * file(s).
 * 
 * If @input_path is a path to a file, the output file will be named the same thing as the
 * input file, but ending with FOUT_EXT instead of .vm. If @input_path is a path to a
 * folder, the output file will have the same name as the input folder, with the file extension FOUT_EXT
 * added. In both cases, the output file will be created in the folder that contains the input file/folder.
 * 
 * The returned code_writer will initially have its in_name field set to NULL.
 *
 * @param input_path the path to the file or folder being translated to assembly
 * @return           the code_writer struct to use for translation
 */
code_writer *VM_Code_Writer(char *input_path) {
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
    code_writer *cw = NULL;

    if (!out) {
        perror("[ERR] Failed to open VM output file");
    } else {
        cw = cw_new(out, NULL, NULL);
    }

    char *bootstrap = vm_write_initial(output_path);
    fprintf(out, "%s\n", bootstrap);

    free(bootstrap);
    free(output_path);
    free(outfile_name);
    free(outfile_base_name);
    path_parts_delete(&input_parts);

    return cw;
}


/**
 * Writes the initial assembly commands needed to bootstrap the program.
 *
 * @param output_path the path to the file that the assembly translation is being written to
 * @return a string containing the bootstrapping assembly code
 */
char *vm_write_initial(char *output_path) {
    int initial_len = fmt_str_len(&INIT) + strlen(output_path);
    char *initial = calloc(initial_len + 1, sizeof(char));

    snprintf(initial, initial_len + 1, INIT.str, output_path);

    return initial;
}


/**
 * Writes a VM command in Hack assembly format.
 *
 * @param command      the VM command to translate
 * @param command_type the command type of the VM command
 * @param cw           the code_writer to use to write the translated command
 * @return             the status of the function
 */
vm_wc_status vm_write_command(char *command, vm_command_t command_type, code_writer *cw) {
    vm_wc_status status = WC_SUCCESS;
    char *arg1 = vm_arg1(command);
    int arg2 = vm_arg2(command);
    char *translated = NULL;

    if (command_type == C_PUSH || command_type == C_POP) {
        if (arg1 != NULL && strlen(arg1) && arg2 > -1) {
            vm_mem_seg seg = seg_to_vm_memseg(arg1);
            translated = vm_write_push_pop(seg, arg2, command_type, cw->in_name);
        } else {
            const char *err = "[ERR] Result of calling vm_arg1() on command \"%s\" was NULL. "
                              "Cannot translate command\n";
            printf(err, command);
            status = WC_INVALID_CMD;
        }
    } else if (command_type == C_ARITHMETIC) {
        translated = vm_write_arithmetic(arg1);
    } else if (command_type == C_LABEL) {
        translated = vm_write_label(cw->func, arg1);
    } else if (command_type == C_GOTO) {
        translated = vm_write_goto(cw->func, arg1);
    } else if (command_type == C_IF) {
        translated = vm_write_if(cw->func, arg1);
    } else if (command_type == C_FUNCTION) {
        cw_set_func(cw, arg1);
        translated = vm_write_function(cw, arg2);
    } else if (command_type == C_RETURN) {
        cw_set_func(cw, NULL);
        translated = vm_write_return();
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

    reinit_str(&translated);
    reinit_str(&arg1);
    return status;
}


/**
 * Encodes arithmetic VM operations as Hack assembly code.
 *
 * @param command the arithmetic command to encode
 * @return        the encoded version of |command|
 */
char *vm_write_arithmetic(char *command) {
    char *goto_label = get_internal_op_label(command);
    int total_len = fmt_str_len(&GOTO_ARITH_OP) + strlen(goto_label) + 2 * num_digits(num_arith_calls);
    char *encoded_goto = calloc(total_len + 1, sizeof(char));
    snprintf(encoded_goto, total_len + 1, GOTO_ARITH_OP.str, num_arith_calls, goto_label, num_arith_calls);

    free(goto_label);
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
char *vm_write_push_pop(vm_mem_seg segment, int index, vm_command_t cmd_type, char *in_fname) {
    char *push_encoded = NULL;
    // The value to push onto the stack is the value starting at the index-th value in 
    // the given segment
    int mem_addr = index + segment.begin_addr;

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
        push_encoded = fmt_str_printf(&PUSH_CONSTANT_SEG, num_digits(mem_addr), mem_addr);
    } else if (segcmp(segment, LCL) || segcmp(segment, ARG) || segcmp(segment, THIS) || segcmp(segment, THAT)) {
        push_encoded = fmt_str_printf(cmd_type == C_PUSH ? &PUSH_VIRTUAL_SEG : &POP_VIRTUAL_SEG,
            strlen(segment.hack_name) + num_digits(index),
            segment.hack_name, index);
    } else if (segcmp(segment, POINTER) || segcmp(segment, TEMP)) {
        push_encoded = fmt_str_printf(cmd_type == C_PUSH ? &PUSH_POINTER_SEG : &POP_POINTER_SEG, num_digits(mem_addr), mem_addr);
    } else if (segcmp(segment, STATIC)) {
        push_encoded = fmt_str_printf(cmd_type == C_PUSH ? &PUSH_STATIC_SEG : &POP_STATIC_SEG, strlen(in_fname) + num_digits(index), in_fname, index);
    } else {
        printf("[ERR] Pushing from the segment %s is not supported\n", segment.vm_name);
    }

    return push_encoded;
}


/**
 * Translates a VM label command into Hack assembly code.
 *
 * @param func  the VM function that the label is in
 * @param label the VM label to define
 * @return      the translated Hack code
 */
char *vm_write_label(char *func, char *label) {
    return gen_label_cmd(DEF_LABEL, func, label);
}


/**
 * Translates a VM goto command into Hack assembly code.
 *
 * @param func  the VM function that the label is in
 * @param label the label to go to
 * @return      the Hack code needed to go to the label
 */
char *vm_write_goto(char *func, char *label) {
    return gen_label_cmd(GOTO_LABEL, func, label);
}


/**
 * Translates a VM if-goto command into Hack assembly code.
 *
 * @param func  the VM function that the label is in
 * @param label the label to conditionally jump to
 * @return      the Hack code needed to conditiionally jump to the label
 */
char *vm_write_if(char *func, char *label) {
    return gen_label_cmd(IF_GOTO_LABEL, func, label);
}


/**
 * Translates a VM function declaration command into Hack assembly code.
 *
 * @param cw       the code_writer struct being used to translate the current .vm file
 * @param num_args the number of arguments the function takes
 * @return         the Hack code needed to define the function in a Hack program
 */
char *vm_write_function(code_writer *cw, int num_args) {
    char *final = NULL;

    if (!valid_identifier(cw->func, 0)) {
        printf("[ERR] vm_write_function was given a code_writer with a func field containing invalid characters\n");
    } else if (cw->func != NULL) {
        final = fmt_str_printf(
            &DEF_FUNC_INIT,
            strlen(cw->func) + num_digits(num_args) + 2 * num_digits(num_func_definitions),
            cw->func, num_func_definitions, num_args, num_func_definitions
        );
        num_func_definitions++;
    } else {
        printf("[ERR] Tried to write function using code_writer with NULL func field\n");
    }

    return final;
}


/**
 * Translates a VM return command into Hack assembly code.
 *
 * @return the Hack code needed to return from a function
 */
char *vm_write_return() {
    return (char*)strdup(FUNC_GOTO_RETURN);
}
}


/**
 * Writes system-generated code to the output file, and closes it.
 *
 * @param cw the code_writer used to write all translated assembly code
 */
void vm_code_writer_close(code_writer *cw) {
    FILE *out = cw->out;

    fprintf(out, "\n// End user-defined program\n\n");
    fprintf(out, "// This terminates the program by sending it into an infinite loop\n");
    fprintf(out, "%s\n", INF_LOOP);
    /*
     * All system-level assembly routines should go after this line. Any assembly routines placed before this
     * may end up being run in the middle of the user's program.
     */

    // Hack routine to initialize a function's local variables
    fprintf(out, "%s\n", INIT_FUNC_LCL);

    // Hack routine to return from a function (including resetting the global stack to the previous state of the caller function)
    fprintf(out, "%s\n", FUNC_RETURN);

    // A map of VM arithmetic operations (add, sub, etc) and the assembly commands associated with them
    ht_hash_table *vm_op_to_asm = ht_new(NUM_ARITH_OPS);
    ht_insert_all(vm_op_to_asm, NUM_ARITH_OPS, ARITHMETIC_OPS, HACK_ARITH_OPS);
    
    // Arithmetic operations (this could be made DRYer, but I think it's more clear when written out)
    char *add_op = gen_arith_cmd(ARITH_ADDSUB_BASE_CMD.str, "add", vm_op_to_asm);
    char *sub_op = gen_arith_cmd(ARITH_ADDSUB_BASE_CMD.str, "sub", vm_op_to_asm);
    char *eq_op = gen_arith_cmd(ARITH_CMP_BASE_CMD.str, "eq", vm_op_to_asm);
    char *gt_op = gen_arith_cmd(ARITH_CMP_BASE_CMD.str, "gt", vm_op_to_asm);
    char *lt_op = gen_arith_cmd(ARITH_CMP_BASE_CMD.str, "lt", vm_op_to_asm);
    char *and_op = gen_arith_cmd(ARITH_BOOL_BASE_CMD.str, "and", vm_op_to_asm);
    char *or_op = gen_arith_cmd(ARITH_BOOL_BASE_CMD.str, "or", vm_op_to_asm);
    char *neg_op = gen_arith_cmd(ARITH_UNARY_BASE_CMD.str, "neg", vm_op_to_asm);
    char *not_op = gen_arith_cmd(ARITH_UNARY_BASE_CMD.str, "not", vm_op_to_asm);
    fprintf(out, "%s\n", add_op);
    fprintf(out, "%s\n", sub_op);
    fprintf(out, "%s\n", eq_op);
    fprintf(out, "%s\n", gt_op);
    fprintf(out, "%s\n", lt_op);
    fprintf(out, "%s\n", and_op);
    fprintf(out, "%s\n", or_op);
    fprintf(out, "%s\n", neg_op);
    fprintf(out, "%s\n", not_op);
    reinit_str(&add_op);
    reinit_str(&sub_op);
    reinit_str(&eq_op);
    reinit_str(&gt_op);
    reinit_str(&lt_op);
    reinit_str(&and_op);
    reinit_str(&or_op);
    reinit_str(&neg_op);
    reinit_str(&not_op);
    

    ht_delete(vm_op_to_asm);

    // Assists jump back to primary program flow after built-in operations
    fprintf(out, "%s\n", JUMP_OP_END);

    // Enables true/false operations
    fprintf(out, "%s\n", TF_FUNC);

    cw_delete(&cw);
}


/**
 * Initializes and returns a code_writer struct. The func field is set to @func.
 *
 * @param outfile     the file to write translated Hack code to
 * @param infile_name the name of the .vm file being read
 * @param func        the initial function name, if any
 * @return            a new code_writer
 */
code_writer *cw_new(FILE *outfile, char *infile_name, char *func) {
    code_writer *cw = calloc(1, sizeof(code_writer));
    cw->out = outfile;

    if (infile_name != NULL) {
        cw->in_name = strdup(infile_name);
    } else {
        cw->in_name = NULL;
    }

    cw_set_func(cw, func);

    return cw;
}


/**
 * Sets the in_name field of a code_writer struct.
 *
 * @param cw      the code_writer on which to set the in_name field
 * @param in_name the new in_name for @cw
 */
void cw_set_in_name(code_writer *cw, char *in_name){
    if (cw->in_name != NULL) {
        reinit_str(&(cw->in_name));
    }

    if (in_name != NULL) {
        cw->in_name = strdup(in_name);
    } else {
        cw->in_name = NULL;
    }
}


/**
 * Sets the func field of a code_writer struct. If the in_name field of @cw is not NULL, the new func field
 * will be set to <@cw.in_name>.<@func>.
 *
 * @param cw   the code_writer on which to set the func field
 * @param func the new func name
 */
void cw_set_func(code_writer *cw, char *func) {
    reinit_str(&(cw->func));

    if (func != NULL) {
        cw->func = strdup(func);
    } else {
        cw->func = NULL;
    }
}


/**
 * Deletes a code_writer struct.
 *
 * @param cw the code_writer to delete
 */
void cw_delete(code_writer **cw) {
    if ((*cw)->out != NULL) {
        fclose((*cw)->out);
        (*cw)->out = NULL;
    }
    if ((*cw)->in_name != NULL) {
        reinit_str(&((*cw)->in_name));
    }
    if ((*cw)->func != NULL) {
        reinit_str(&((*cw)->func));
    }
    if (*cw != NULL) {
        free(*cw);
        *cw = NULL;
    }
}
