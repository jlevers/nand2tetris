#ifndef _VM_ASM_CONSTANTS
#define _VM_ASM_CONSTANTS

/*
 * NOTES
 * -----
 * For arithmetic operations, the address to jump to after completing the operation is stored in @R13.
 * For true/false operations, the address to jump to after completing the operation is stored in @R14.
 * Intermediate pointers in pop operations are stored in @R15.
 * All internal labels begin with a double underscore: __LABEL_NAME
 */

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


// All (PUSH|POP)_XXX_SEG constants are series of .hack commands for pushing/popping to/from the stack
const char *PUSH_CONSTANT_SEG =
    "@SP\n"
    "M=M+1\n"
    "@%d\n"
    "D=A\n"
    "@SP\n"
    "A=M-1\n"
    "M=D\n";

const char *PUSH_VIRTUAL_SEG =
    "@%s\n"
    "D=M\n"
    "@%d\n"
    "A=D+A\n"
    "D=M\n"
    "@SP\n"
    "M=M+1\n"
    "A=M-1\n"
    "M=D\n";

const char *POP_VIRTUAL_SEG =
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
    "M=D\n";

const char *PUSH_POINTER_SEG =
    "@%d\n"
    "D=M\n"
    "@SP\n"
    "M=M+1\n"
    "A=M-1\n"
    "M=D\n";

const char *POP_POINTER_SEG =
    "@SP\n"
    "AM=M-1\n"
    "D=M\n"
    "@%d\n"
    "M=D\n";

const char *PUSH_STATIC_SEG =
    "@%s.%d\n"
    "D=M\n"
    "@SP\n"
    "M=M+1\n"
    "A=M-1\n"
    "M=D\n";

const char *POP_STATIC_SEG =
    "@SP\n"
    "AM=M-1\n"
    "D=M\n"
    "@%s.%d\n"
    "M=D\n";

#endif