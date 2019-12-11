#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "minunit.h"
#include "parser.h"
#include "code_writer.h"

int tests_run = 0;

/**
 * Frees a char pointer and sets it to NULL
 * @param char**  to_process  A pointer to the pointer to process
 */
void reinit_char(char **to_process) {
    free(*to_process);
    *to_process = NULL;
}

int same_file(int fd1, int fd2) {
    struct stat stat1, stat2;
    if (fstat(fd1, &stat1) < 0) return -1;
    if (fstat(fd2, &stat2) < 0) return -1;
    return (stat1.st_dev == stat2.st_dev) && (stat1.st_ino == stat2.st_ino);
}

static char *test_parser() {
    // Test Parser()
    char *fin_name = "./src/test/Test.vm";
    FILE *parser_in = VM_Parser(fin_name);
    mu_assert("parser_new does not correctly open input file",
        same_file(fileno(parser_in), open(fin_name, 'r')));

    // Test vm_advance()
    char *line = vm_advance(parser_in);
    mu_assert("vm_advance did not skip full-line comment and/or empty line",
        !strcmp(line, "push constant 7"));
    reinit_char(&line);
    line = vm_advance(parser_in);
    mu_assert("vm_advance did not properly ignore inline comment and/or consecutive spaces",
        !strcmp(line, "push constant 8"));
    reinit_char(&line);

    line = vm_advance(parser_in);
    reinit_char(&line);
    line = vm_advance(parser_in);
    mu_assert("vm_advance did not return NULL at EOF", line == NULL);
    reinit_char(&line);

    fclose(parser_in);

    // Test vm_command_type()
    mu_assert("vm_command_type did not return C_INVALID when given an invalid command",
        vm_command_type("asdf") == C_INVALID);
    mu_assert("vm_command_type did not return C_PUSH when given command `push constant 7`",
        vm_command_type("push constant 7") == C_PUSH);
    mu_assert("vm_command_type did not return C_POP when given command `pop local 0`",
        vm_command_type("pop local 0") == C_POP);
    mu_assert("vm_command_type did not return C_LABEL when given command `label test`",
        vm_command_type("label test") == C_LABEL);
    mu_assert("vm_command_type did not return C_GOTO when given command `goto asdf`",
        vm_command_type("goto asdf") == C_GOTO);
    mu_assert("vm_command_type did not return C_IF when given command `if-goto end`",
        vm_command_type("if-goto end") == C_IF);
    mu_assert("vm_command_type did not return C_FUNCTION when given command `function mult 2`",
        vm_command_type("function mult 2") == C_FUNCTION);
    mu_assert("vm_command_type did not return C_RETURN when given command `return`",
        vm_command_type("return") == C_RETURN);
    mu_assert("vm_command_type did not return C_ARITHMETIC when given command `add`",
        vm_command_type("add") == C_ARITHMETIC);
    mu_assert("vm_command_type did not return C_ARITHMETIC when given command `sub`",
        vm_command_type("sub") == C_ARITHMETIC);
    mu_assert("vm_command_type did not return C_ARITHMETIC when given command `neg`",
        vm_command_type("neg") == C_ARITHMETIC);
    mu_assert("vm_command_type did not return C_ARITHMETIC when given command `eq`",
        vm_command_type("eq") == C_ARITHMETIC);
    mu_assert("vm_command_type did not return C_ARITHMETIC when given command `gt`",
        vm_command_type("gt") == C_ARITHMETIC);
    mu_assert("vm_command_type did not return C_ARITHMETIC when given command `lt`",
        vm_command_type("lt") == C_ARITHMETIC);
    mu_assert("vm_command_type did not return C_ARITHMETIC when given command `and`",
        vm_command_type("and") == C_ARITHMETIC);
    mu_assert("vm_command_type did not return C_ARITHMETIC when given command `or`",
        vm_command_type("or") == C_ARITHMETIC);
    mu_assert("vm_command_type did not return C_ARITHMETIC when given command `not`",
        vm_command_type("not") == C_ARITHMETIC);


    // Test vm_arg1()
    char *arg1_invalid = vm_arg1("asdf");
    char *arg1_ret = vm_arg1("return");
    char *arg1_arith1 = vm_arg1("add");
    char *arg1_arith2 = vm_arg1("lt");
    char *arg1_goto = vm_arg1("goto start");
    char *arg1_ifgoto = vm_arg1("if-goto end");
    char *arg1_push = vm_arg1("push local 0");
    char *arg1_label = vm_arg1("label test");
    char *arg1_func = vm_arg1("function mult 2");
    mu_assert("vm_arg1 did not return NULL when given an invalid command", arg1_invalid == NULL);
    mu_assert("vm_arg1 did not return NULL when given a command of type C_RETURN", arg1_ret == NULL);
    mu_assert("vm_arg1 did not return `add` when given the C_ARITHMETIC command `add`",
        !strcmp(arg1_arith1, "add"));
    mu_assert("vm_arg1 did not return `lt` when given the C_ARITHMETIC command `lt`",
        !strcmp(arg1_arith2, "lt"));
    mu_assert("vm_arg1 did not return `start` when given the command `goto start`",
        !strcmp(arg1_goto, "start"));
    mu_assert("vm_arg1 did not return `end` when given the command `if-goto end`",
        !strcmp(arg1_ifgoto, "end"));
    mu_assert("vm_arg1 did not return `local` when given the command `push local 0`",
        !strcmp(arg1_push, "local"));
    mu_assert("vm_arg1 did not return `test` when given the command `label test`",
        !strcmp(arg1_label, "test"));
    mu_assert("vm_arg1 did not return `mult` when given the command `function mult 2`",
        !strcmp(arg1_func, "mult"));
    reinit_char(&arg1_invalid);
    reinit_char(&arg1_ret);
    reinit_char(&arg1_arith1);
    reinit_char(&arg1_arith2);
    reinit_char(&arg1_goto);
    reinit_char(&arg1_ifgoto);
    reinit_char(&arg1_push);
    reinit_char(&arg1_label);
    reinit_char(&arg1_func);


    // Test vm_arg2()
    int arg2_invalid = vm_arg2("asdf");
    int arg2_arith = vm_arg2("add");
    int arg2_goto = vm_arg2("goto start");
    int arg2_ifgoto = vm_arg2("if-goto end");
    int arg2_label = vm_arg2("label test");
    int arg2_push = vm_arg2("push constant 0");
    int arg2_pop = vm_arg2("pop that 2");
    int arg2_func = vm_arg2("function mult 33");
    int arg2_call = vm_arg2("call mult 771");
    mu_assert("vm_arg2 did not return -1 when given an invalid command", arg2_invalid == -1);
    mu_assert("vm_arg2 did not return -1 when given an arithmetic command", arg2_arith == -1);
    mu_assert("vm_arg2 did not return -1 when given a goto command", arg2_goto == -1);
    mu_assert("vm_arg2 did not return -1 when given a if-goto command", arg2_ifgoto == -1);
    mu_assert("vm_arg2 did not return -1 when given a label command", arg2_label == -1);
    mu_assert("vm_arg2 did not return 0 when given the command `push constant 0`", arg2_push == 0);
    mu_assert("vm_arg2 did not return 2 when given the command `pop that 2`", arg2_pop == 2);
    mu_assert("vm_arg2 did not return 33 when given the command `function mult 33`",
        arg2_func == 33);
    mu_assert("vm_arg2 did not return 771 when given the command `call mult 771`",
        arg2_call == 771);


    return 0;
}

static char* test_code_writer() {
    // Test VM_Code_Writer()
    char *file_input_path = "./src/test/Test.vm";
    FILE *file_output_file = VM_Code_Writer(file_input_path);
    mu_assert("VM_Code_Writer did not open the correct output file given an file path as input",
        same_file(fileno(file_output_file), open("./src/test/Test.asm", 'r')));

    char *folder_input_path = "./src/test/TestDir/";
    FILE *folder_output_file = VM_Code_Writer(folder_input_path);
    mu_assert("VM_Code_Writer did not open the correct output file given a folder path as input",
        same_file(fileno(folder_output_file), open("./src/test/TestDir.asm", 'a')));
    fclose(folder_output_file);


    // Test vm_write_command()
    mu_assert("vm_write_command did not return WC_INVALID_CMD when given a command of type C_INVALID",
        vm_write_command("asdf", C_INVALID, file_output_file) == WC_INVALID_CMD);
    mu_assert("vm_write_command did not return WC_INVALID_CMD when given a command of type C_PUSH with no arguments",
        vm_write_command("push", C_PUSH, file_output_file) == WC_INVALID_CMD);
    mu_assert("vm_write_command did not return WC_INVALID_CMD when given a command of type C_POP with a negative index",
        vm_write_command("pop local -1", C_POP, file_output_file) == WC_INVALID_CMD);
    mu_assert("vm_write_command did not return WC_SUCCESS when given a valid C_PUSH command",
        vm_write_command("push constant 2", C_PUSH, file_output_file) == WC_SUCCESS);
    mu_assert("vm_write_command did not return WC_SUCCESS when given a valid C_POP command",
        vm_write_command("pop local 1", C_POP, file_output_file) == WC_SUCCESS);
    mu_assert("vm_write_command did not return WC_SUCCESS when given a valid C_ARITHMETIC command",
        vm_write_command("add", C_ARITHMETIC, file_output_file) == WC_SUCCESS);
    mu_assert("vm_write_command did not return WC_UNSUPPORTED_CMD when given a command of type C_LABEL",
        vm_write_command("label test", C_LABEL, file_output_file) == WC_UNSUPPORTED_CMD);
    mu_assert("vm_write_command did not return WC_UNSUPPORTED_CMD when given a command of type C_GOTO",
        vm_write_command("goto func", C_GOTO, file_output_file) == WC_UNSUPPORTED_CMD);
    mu_assert("vm_write_command did not return WC_UNSUPPORTED_CMD when given a command of type C_IF",
        vm_write_command("if-goto end", C_IF, file_output_file) == WC_UNSUPPORTED_CMD);
    mu_assert("vm_write_command did not return WC_UNSUPPORTED_CMD when given a command of type C_FUNCTION",
        vm_write_command("function mult 2", C_FUNCTION, file_output_file) == WC_UNSUPPORTED_CMD);
    mu_assert("vm_write_command did not return WC_UNSUPPORTED_CMD when given a command of type C_RETURN",
        vm_write_command("return", C_RETURN, file_output_file) == WC_UNSUPPORTED_CMD);
    mu_assert("vm_write_command did not return WC_UNSUPPORTED_CMD when given a command of type C_CALL",
        vm_write_command("call mult 2", C_CALL, file_output_file) == WC_UNSUPPORTED_CMD);

    fclose(file_output_file);


    // Test vm_translate_push()
    // const char *translate_push_temp_3 = vm_translate_push(TEMP, 3);
    char *translate_push_constant_59 = vm_translate_push(CONSTANT, 59);

    mu_assert("vm_translate_push did not return NULL when given memory segment SEG_INVALID",
        vm_translate_push(SEG_INVALID, 2) == NULL);
    mu_assert("vm_translate_push did not return the correct set of commands given index 59 of segment CONSTANT",
        !strcmp(translate_push_constant_59,
            "@SP\n"
            "M=M+1\n"
            "@59\n"
            "D=A\n"
            "@SP\n"
            "A=M-1\n"
            "M=D\n"));
    // mu_assert("vm_translate_push did not return the correct set of commands given index 3 of segment TEMP",
        // !strcmp(translate_push_temp_3, "@SP\nA=A+1\n@8\nD=M\n@SP\nM=D\n"));
    mu_assert("vm_translate_push did not return NULL when given segment TEMP", vm_translate_push(TEMP, 3) == NULL);
    mu_assert("vm_translate_push did not return NULL when given segment TEMP and an out-of-bounds index",
        vm_translate_push(TEMP, 9) == NULL);

    reinit_char(&translate_push_constant_59);


    // Test vm_translate_arithmetic()
    char *translate_add = vm_translate_arithmetic("add");
    mu_assert("vm_translate_arithmetic doesn't jump to the \"add\" command correctly",
        !strcmp(translate_add,
            "@POST_ARITH_CALL_1\n"
            "D=A\n"
            "@R13\n"
            "M=D\n"
            "@__ADD_OP\n"
            "0;JMP\n"
            "(POST_ARITH_CALL_1)\n"));

    reinit_char(&translate_add);

    return 0;
}

static char *all_tests() {
    mu_run_test(test_parser);
    mu_run_test(test_code_writer);
    return 0;
}

int main() {
    printf("[TEST] Begin test output\n");
    printf("------------------------\n\n");

    char *result = all_tests();
    if (result != 0) {
        printf("[TEST] %s\n", result);
    } else {
        printf("\n[TEST] ALL TESTS PASSED\n");
    }

    printf("[TEST] Tests run: %d\n", tests_run);
    printf("\n----------------------\n");
    printf("[TEST] End test output\n");

    return result != 0;
}
