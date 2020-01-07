#include <stdlib.h>

#include "asm_constants.h"
#include "util.h"

/*
 * NOTES
 * -----
 * @R13:
 *  - Stores the address to jump to after completing a built-in operation.
 *
 * @R14:
 *  - Stores the address to jump to after completing a true/false operation.
 *  - Stores the number of local variables that need to be initialized when creating function definitions.
 *
 * @R15:
 *  - Stores intermediate pointers in pop operations.
 *
 * TEMP[0]:
 *  - Stores the number of local variables to initialize when calling a function
 *  - Stores a pointer to the frame of a returning function
 *  - Stores the return address for a called function, which is set by the caller function
 * TEMP[1]:
 *  - Stores the addresses of local variables while they're being initialized to 0
 *  - When returning from a function, stores the return address to return to
 *  - Stores the number of arguments pushed onto the stack for function that's being called
 *
 * All internal procedure labels begin with a double underscore: __LABEL_NAME
 * All internal variable labels begin and end with a double underscore: __var__
 */

const fmt_str INIT = {
    .str =
        "// Generated by Nand2Tetris VM translator (written by Jesse Evers)\n"
        "// File: %s\n\n"
        "// Initialize stack pointer\n"
        "@256\n"
        "D=A\n"
        "@SP\n"
        "M=D\n\n"
        "@Sys.init\n"
        "0;JMP\n\n"
        "// Begin user-defined program\n",
    .fmt_len = 2
};

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
    "0;JMP\n";

const fmt_str GOTO_ARITH_OP = {
    .str =
        "@POST_ARITH_CALL_%d\n"
        "D=A\n"
        "@R13\n"
        "M=D\n"
        "@%s\n"
        "0;JMP\n"
        "(POST_ARITH_CALL_%d)\n",
    .fmt_len = 6
};

const char *JUMP_OP_END =
    "(__END_OP)\n"
    "@R13\n"
    "A=M\n"
    "0;JMP\n";

/*
 * All the following ARITH_<OPTYPE>_BASE_CMD constants are missing their function labels, which are generated
 * in gen_arith_cmd() when the command is written to the end of the assembly file. Each base command contains a single
 * format specifier in the place of a specific arithmetic operator, which will be filled in by gen_arith_cmd().
 */
const fmt_str ARITH_ADDSUB_BASE_CMD = {
    .str =
        "@SP\n"
        "A=M-1\n"
        "D=M\n"
        "@SP\n"
        "M=M-1\n"
        "A=M-1\n"
        "D=M%sD\n"
        "M=D\n"
        "@__END_OP\n"
        "0;JMP\n",
    .fmt_len = 2
};

const fmt_str ARITH_CMP_BASE_CMD = {
    .str =
        "@SP\n"
        "A=M-1\n"
        "D=M\n"
        "@SP\n"
        "M=M-1\n"
        "A=M-1\n"
        "M=M-D\n"
        "@__END_OP\n"
        "D=A\n"
        "@R14\n"
        "M=D\n"
        "@SP\n"
        "A=M-1\n"
        "D=M\n"
        "@__TRUE\n"
        "D;J%s\n"
        "@__FALSE\n"
        "0;JMP\n",
    .fmt_len = 2
};

const fmt_str ARITH_BOOL_BASE_CMD = {
    .str =
        "@SP\n"
        "A=M-1\n"
        "D=M\n"
        "@SP\n"
        "M=M-1\n"
        "A=M-1\n"
        "M=M%sD\n"
        "@__END_OP\n"
        "0;JMP\n",
    .fmt_len = 2
};

const fmt_str ARITH_UNARY_BASE_CMD = {
    .str =
        "@SP\n"
        "A=M-1\n"
        "M=%sM\n"
        "@__END_OP\n"
        "0;JMP\n",
    .fmt_len = 2
};

const int NUM_ARITH_OPS = 9;
const char *HACK_ARITH_OPS[] = {"+", "-", "-", "EQ", "GT", "LT", "&", "|", "!", NULL};



// All (PUSH|POP)_XXX_SEG constants are series of Hack commands for pushing/popping to/from the stack
const fmt_str PUSH_CONSTANT_SEG = {
    .str =
        "@SP\n"
        "M=M+1\n"
        "@%d\n"
        "D=A\n"
        "@SP\n"
        "A=M-1\n"
        "M=D\n",
    .fmt_len = 2
};

const fmt_str PUSH_VIRTUAL_SEG = {
    .str =
        "@%s\n"
        "D=M\n"
        "@%d\n"
        "A=D+A\n"
        "D=M\n"
        "@SP\n"
        "M=M+1\n"
        "A=M-1\n"
        "M=D\n",
    .fmt_len = 4
};

const fmt_str POP_VIRTUAL_SEG = {
    .str =
        "@%s\n"
        "D=M\n"
        "@%d\n"
        "D=D+A\n"
        "@R15\n"
        "M=D\n"
        "@SP\n"
        "AM=M-1\n"
        "D=M\n"
        "@R15\n"
        "A=M\n"
        "M=D\n",
    .fmt_len = 4
};

const fmt_str PUSH_POINTER_SEG = {
    .str =
        "@%d\n"
        "D=M\n"
        "@SP\n"
        "M=M+1\n"
        "A=M-1\n"
        "M=D\n",
    .fmt_len = 2
};

const fmt_str POP_POINTER_SEG = {
    .str =
        "@SP\n"
        "AM=M-1\n"
        "D=M\n"
        "@%d\n"
        "M=D\n",
    .fmt_len = 2
};

const fmt_str PUSH_STATIC_SEG = {
    .str =
        "@%s.%d\n"
        "D=M\n"
        "@SP\n"
        "M=M+1\n"
        "A=M-1\n"
        "M=D\n",
    .fmt_len = 4
};

const fmt_str POP_STATIC_SEG = {
    .str =
        "@SP\n"
        "AM=M-1\n"
        "D=M\n"
        "@%s.%d\n"
        "M=D\n",
    .fmt_len = 4
};



const fmt_str DEF_LABEL = {
    .str = "(%s%c%s)\n",
    .fmt_len = 6
};

const fmt_str GOTO_LABEL = {
    .str =
        "@%s%c%s\n"
        "0;JMP\n",
    .fmt_len = 6
};

const fmt_str IF_GOTO_LABEL = {
    .str =
        "@SP\n"
        "AM=M-1\n"
        "D=M\n"
        "@%s%c%s\n"
        "D;JNE\n",
    .fmt_len = 6
};

const fmt_str DEF_FUNC_INIT = {
    .str =
        "(%s)\n"  // The filled-in version of DEF_FUNC_LABEL
        // Store the address to jump back to after initializing the function in R13
        "@FUNC_DEF_%d\n"
        "D=A\n"
        "@R13\n"
        "M=D\n"
        // Store the number of local variables to initialize in TEMP[0]
        "@%d\n"
        "D=A\n"
        "@TEMP\n"
        "M=D\n"
        // Jump to the local variables initialization procedure
        "@__INIT_LCL\n"
        "0;JMP\n"
        // A label for the main body of the function
        "(FUNC_DEF_%d)\n",
    .fmt_len = 8
};

const char *INIT_FUNC_LCL =
    "(__INIT_LCL)\n"
    // Get the number of local variables to initialize from TEMP[0]
    "@TEMP\n"
    "D=M\n"
    // If there are no local variables in the function, jump back to the function definition
    "@__END_OP\n"
    "D;JEQ\n"
    "(__INIT_LCL_LOOP)\n"
    // Make the address of the Nth local variable equal to (@LCL + N - 1) since addressing is zero-indexed
    "@LCL\n"
    "D=M+D\n"
    "D=D-1\n"
    // Store this local variable's address in TEMP[1]
    "@TEMP\n"
    "A=A+1\n"
    "M=D\n"
    // Set the local variable to 0
    "@0\n"
    "D=A\n"
    "@TEMP\n"
    "A=A+1\n"
    "A=M\n"
    "M=D\n"
    // Decrement how many local variables need to be initialized
    "@TEMP\n"
    "MD=M-1\n"
    // Keep looping if there are still variables to initialize
    "@__INIT_LCL_LOOP\n"
    "D;JGT\n"
    // Jump back to the function definition
    "@__END_OP\n"
    "0;JMP\n";

const char *FUNC_GOTO_RETURN =
    "@__FUNC_RETURN\n"
    "0;JMP\n";

const char *FUNC_RETURN =
    "(__FUNC_RETURN)\n"
    // Store the returning function's frame in TEMP[0]
    "@LCL\n"
    "D=M\n"
    "@TEMP\n"
    "M=D\n"
    // Store the return address of the caller function in TEMP[1]
    "@5\n"
    "A=D-A\n"
    "D=M\n"
    "@TEMP\n"
    "A=A+1\n"
    "M=D\n"
    // Make ARG[0] equal to the return value, which is the top item on the stack
    "@SP\n"
    "A=M-1\n"
    "D=M\n"
    "@ARG\n"
    "A=M\n"
    "M=D\n"
    // Set SP to one address ahead of the return value
    "@ARG\n"
    "D=M\n"
    "@SP\n"
    "M=D+1\n"
    // Reset THAT to be the caller function's THAT address
    "@TEMP\n"
    "AM=M-1\n"
    "D=M\n"
    "@THAT\n"
    "M=D\n"
    // Reset THIS to be the caller function's THIS address
    "@TEMP\n"
    "AM=M-1\n"
    "D=M\n"
    "@THIS\n"
    "M=D\n"
    // Reset ARG to be the caller function's ARG address
    "@TEMP\n"
    "AM=M-1\n"
    "D=M\n"
    "@ARG\n"
    "M=D\n"
    // Rest LCL to be the caller function's LCL address
    "@TEMP\n"
    "AM=M-1\n"
    "D=M\n"
    "@LCL\n"
    "M=D\n"
    // Retrieve the return address of the caller function and jump to it
    "@TEMP\n"
    "A=A+1\n"
    "A=M\n"
    "0;JMP\n";

