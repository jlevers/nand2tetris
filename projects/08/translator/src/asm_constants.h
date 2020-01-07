#ifndef _VM_ASM_CONSTANTS_H
#define _VM_ASM_CONSTANTS_H

#include "util.h"

// General use
extern const fmt_str INIT;
extern const char *INF_LOOP;
extern const char *TF_FUNC;

// Related to arithmetic operations
extern const fmt_str GOTO_ARITH_OP;
extern const char *JUMP_OP_END;
extern const fmt_str ARITH_ADDSUB_BASE_CMD;
extern const fmt_str ARITH_CMP_BASE_CMD;
extern const fmt_str ARITH_BOOL_BASE_CMD;
extern const fmt_str ARITH_UNARY_BASE_CMD;
extern const int NUM_ARITH_OPS;
extern const char *HACK_ARITH_OPS[];

// Related to push/pop operations
extern const fmt_str PUSH_CONSTANT_SEG;
extern const fmt_str POP_CONSTANT_SEG;
extern const fmt_str PUSH_VIRTUAL_SEG;
extern const fmt_str POP_VIRTUAL_SEG;
extern const fmt_str PUSH_POINTER_SEG;
extern const fmt_str POP_POINTER_SEG;
extern const fmt_str PUSH_STATIC_SEG;
extern const fmt_str POP_STATIC_SEG;

// Related to control flow operations
extern const fmt_str DEF_LABEL;
extern const fmt_str GOTO_LABEL;
extern const fmt_str IF_GOTO_LABEL;

// Related to functions
extern const fmt_str DEF_FUNC_INIT;
extern const char *INIT_FUNC_LCL;
extern const char *FUNC_GOTO_RETURN;
extern const char *FUNC_RETURN;

#endif /* _VM_ASM_CONSTANTS_H */
