// Generated by Nand2Tetris VM translator (written by Jesse Evers)
// File: ../FunctionCalls/SimpleFunction.asm

// Initialize stack pointer
//@256
//D=A
//@SP
//M=D

// Save context before Sys.init call
//@__PRE_FUNC_CALL_INIT
//D=A
//@R13
//M=D
//@__FUNC_CALL
//0;JMP
//(__PRE_FUNC_CALL_INIT)
//@Sys.init
//0;JMP

// Begin user-defined program

(SimpleFunction.test)
@__FUNC_DEF_0
D=A
@R13
M=D
@2
D=A
@R14
M=D
@__INIT_LCL
0;JMP
(__FUNC_DEF_0)
@LCL
D=M
@0
A=D+A
D=M
@SP
M=M+1
A=M-1
M=D
@LCL
D=M
@1
A=D+A
D=M
@SP
M=M+1
A=M-1
M=D
@POST_ARITH_CALL_0
D=A
@R13
M=D
@__ADD_OP
0;JMP
(POST_ARITH_CALL_0)
@POST_ARITH_CALL_1
D=A
@R13
M=D
@__NOT_OP
0;JMP
(POST_ARITH_CALL_1)
@ARG
D=M
@0
A=D+A
D=M
@SP
M=M+1
A=M-1
M=D
@POST_ARITH_CALL_2
D=A
@R13
M=D
@__ADD_OP
0;JMP
(POST_ARITH_CALL_2)
@ARG
D=M
@1
A=D+A
D=M
@SP
M=M+1
A=M-1
M=D
@POST_ARITH_CALL_3
D=A
@R13
M=D
@__SUB_OP
0;JMP
(POST_ARITH_CALL_3)
@__FUNC_RETURN
0;JMP

// End user-defined program

// This terminates the program by sending it into an infinite loop
(__INFINITE_LOOP)
@__INFINITE_LOOP
0;JMP

(__FUNC_CALL)
@5
D=M
@SP
M=M+1
A=M-1
M=D
@LCL
D=M
@SP
M=M+1
A=M-1
M=D
@ARG
D=M
@SP
M=M+1
A=M-1
M=D
@THIS
D=M
@SP
M=M+1
A=M-1
M=D
@THAT
D=M
@SP
M=M+1
A=M-1
M=D
@SP
D=M
@5
D=D-A
@5
A=A+1
D=D-M
@ARG
M=D
@SP
D=M
@LCL
M=D
@__END_OP
0;JMP

(__INIT_LCL)
@R14
D=M
@__END_OP
D;JEQ
(__INIT_LCL_LOOP)
@LCL
D=M+D
A=D-1
M=0
@R14
MD=M-1
@__INIT_LCL_LOOP
D;JGT
@__END_OP
0;JMP

(__FUNC_RETURN)
@LCL
D=M
@5
M=D
@5
A=D-A
D=M
@5
A=A+1
M=D
@SP
A=M-1
D=M
@ARG
A=M
M=D
@ARG
D=M
@SP
M=D+1
@5
AM=M-1
D=M
@THAT
M=D
@5
AM=M-1
D=M
@THIS
M=D
@5
AM=M-1
D=M
@ARG
M=D
@5
AM=M-1
D=M
@LCL
M=D
@5
A=A+1
A=M
0;JMP

(__ADD_OP)
@SP
A=M-1
D=M
@SP
M=M-1
A=M-1
D=M+D
M=D
@__END_OP
0;JMP

(__SUB_OP)
@SP
A=M-1
D=M
@SP
M=M-1
A=M-1
D=M-D
M=D
@__END_OP
0;JMP

(__EQ_OP)
@SP
A=M-1
D=M
@SP
M=M-1
A=M-1
M=M-D
@__END_OP
D=A
@R14
M=D
@SP
A=M-1
D=M
@__TRUE
D;JEQ
@__FALSE
0;JMP

(__GT_OP)
@SP
A=M-1
D=M
@SP
M=M-1
A=M-1
M=M-D
@__END_OP
D=A
@R14
M=D
@SP
A=M-1
D=M
@__TRUE
D;JGT
@__FALSE
0;JMP

(__LT_OP)
@SP
A=M-1
D=M
@SP
M=M-1
A=M-1
M=M-D
@__END_OP
D=A
@R14
M=D
@SP
A=M-1
D=M
@__TRUE
D;JLT
@__FALSE
0;JMP

(__AND_OP)
@SP
A=M-1
D=M
@SP
M=M-1
A=M-1
M=M&D
@__END_OP
0;JMP

(__OR_OP)
@SP
A=M-1
D=M
@SP
M=M-1
A=M-1
M=M|D
@__END_OP
0;JMP

(__NEG_OP)
@SP
A=M-1
M=-M
@__END_OP
0;JMP

(__NOT_OP)
@SP
A=M-1
M=!M
@__END_OP
0;JMP

(__END_OP)
@R13
A=M
0;JMP

(__TRUE)
@SP
A=M-1
M=-1
@R14
A=M
0;JMP
(__FALSE)
@SP
A=M-1
M=0
@R14
A=M
0;JMP

