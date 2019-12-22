#ifndef _VM_ASM_CONSTANTS_H
#define _VM_ASM_CONSTANTS_H

// .hack assembly snippets
extern const char *INIT;
extern const char *INF_LOOP;
extern const char *TF_FUNC;
extern const char *GOTO_ARITH_OP;
extern const char *ARITH_OP_END;
extern const char *ARITH_ADDSUB_BASE_CMD;
extern const char *ARITH_CMP_BASE_CMD;
extern const char *ARITH_BOOL_BASE_CMD;
extern const char *ARITH_UNARY_BASE_CMD;
extern const char *PUSH_CONSTANT_SEG;
extern const char *POP_CONSTANT_SEG;
extern const char *PUSH_VIRTUAL_SEG;
extern const char *POP_VIRTUAL_SEG;
extern const char *PUSH_POINTER_SEG;
extern const char *POP_POINTER_SEG;
extern const char *PUSH_STATIC_SEG;
extern const char *POP_STATIC_SEG;

extern const int NUM_ARITH_OPS;
extern const char *ASM_OPS[];

#endif /* _VM_ASM_CONSTANTS_H */
