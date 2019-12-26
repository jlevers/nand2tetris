#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "code_writer.h"
#include "minunit.h"
#include "parser.h"
#include "util.h"

int tests_run = 0;


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

static char *test_code_writer() {
    // Test cw_new()
    FILE *test_out = fopen("./src/test/TestOut.asm", "a");  // This doesn't need to be closed since it's closed by cw_delete
    code_writer *cw1 = cw_new(test_out, "Asdf");
    code_writer *cw2 = cw_new(NULL, NULL);

    mu_assert("cw_new did not correctly set the output file when given an open file handle",
        same_file(fileno(cw1->out), open("./src/test/TestOut.asm", 'r')));
    mu_assert("cw_new did not set the output file to NULL when given a NULL file handle", cw2->out == NULL);
    mu_assert("cw_new did not correctly set the code_writer's in_file", !strcmp(cw1->in_name, "Asdf"));
    mu_assert("cw_new did not set the code_writer's in_file to NULL when given a NULL in_file arg", cw2->in_name == NULL);
    mu_assert("cw_new did not set the code_writer's function to the default correctly", !strcmp(cw1->func, "default"));


    // Test cw_set_in_name()
    cw_set_in_name(cw1, "qwer");
    cw_set_in_name(cw2, "something");

    mu_assert("cw_set_in_name did not correctly set an already-defined in_file field", !strcmp(cw1->in_name, "qwer"));
    mu_assert("cw_set_in_name did not correctly set an initially NULL in_file field", !strcmp(cw2->in_name, "something"));
    cw_set_in_name(cw1, NULL);
    mu_assert("cw_set_in_name did not correctly set an in_file field to NULL", cw1->in_name == NULL);


    // Test cw_set_func
    cw_set_func(cw1, "foo");
    cw_set_func(cw2, NULL);

    mu_assert("cw_set_func did not correctly set an already-defined func field", !strcmp(cw1->func, "foo"));
    mu_assert("cw_set_func did not correctly set a func field to NULL", cw2->func == NULL);
    cw_set_func(cw2, "def");
    mu_assert("cw_set_func did not correctly set a NULL func field to a non-NULL value", !strcmp(cw2->func, "def"));


    // Test cw_delete()
    cw_delete(&cw1);
    cw_delete(&cw2);

    mu_assert("cw_delete did not fully delete test code_writers cw1 and cw2", cw1 == NULL && cw2 == NULL);


    // Test VM_Code_Writer()
    const char *file_path = "./src/test/Test.vm";
    const char *folder_path = "./src/test/TestDir/";
    char *heap_file_path = strdup(file_path);
    char *heap_folder_path = strdup(folder_path);

    code_writer *file_code_writer = VM_Code_Writer(heap_file_path);
    code_writer *folder_code_writer = VM_Code_Writer(heap_folder_path);
    cw_set_in_name(file_code_writer, "Test");

    mu_assert("VM_Code_Writer did not open the correct output file given an file path as input",
        same_file(fileno(file_code_writer->out), open("./src/test/Test.asm", 'r')));
    mu_assert("VM_Code_Writer did not open the correct output file given a folder path as input",
        same_file(fileno(folder_code_writer->out), open("./src/test/TestDir.asm", 'a')));

    free(heap_folder_path);
    free(heap_file_path);

    vm_code_writer_close(folder_code_writer);

    // Test vm_write_command()
    mu_assert("vm_write_command did not return WC_INVALID_CMD when given a command of type C_INVALID",
        vm_write_command("asdf", C_INVALID, file_code_writer) == WC_INVALID_CMD);
    mu_assert("vm_write_command did not return WC_INVALID_CMD when given a command of type C_PUSH with no arguments",
        vm_write_command("push", C_PUSH, file_code_writer) == WC_INVALID_CMD);
    mu_assert("vm_write_command did not return WC_INVALID_CMD when given a command of type C_POP with a negative index",
        vm_write_command("pop local -1", C_POP, file_code_writer) == WC_INVALID_CMD);
    mu_assert("vm_write_command did not return WC_SUCCESS when given a valid C_PUSH command",
        vm_write_command("push constant 2", C_PUSH, file_code_writer) == WC_SUCCESS);
    mu_assert("vm_write_command did not return WC_SUCCESS when given a valid C_POP command",
        vm_write_command("pop local 1", C_POP, file_code_writer) == WC_SUCCESS);
    mu_assert("vm_write_command did not return WC_SUCCESS when given a valid C_ARITHMETIC command",
        vm_write_command("add", C_ARITHMETIC, file_code_writer) == WC_SUCCESS);
    mu_assert("vm_write_command did not return WC_UNSUPPORTED_CMD when given a command of type C_LABEL",
        vm_write_command("label test", C_LABEL, file_code_writer) == WC_UNSUPPORTED_CMD);
    mu_assert("vm_write_command did not return WC_UNSUPPORTED_CMD when given a command of type C_GOTO",
        vm_write_command("goto func", C_GOTO, file_code_writer) == WC_UNSUPPORTED_CMD);
    mu_assert("vm_write_command did not return WC_UNSUPPORTED_CMD when given a command of type C_IF",
        vm_write_command("if-goto end", C_IF, file_code_writer) == WC_UNSUPPORTED_CMD);
    mu_assert("vm_write_command did not return WC_UNSUPPORTED_CMD when given a command of type C_FUNCTION",
        vm_write_command("function mult 2", C_FUNCTION, file_code_writer) == WC_UNSUPPORTED_CMD);
    mu_assert("vm_write_command did not return WC_UNSUPPORTED_CMD when given a command of type C_RETURN",
        vm_write_command("return", C_RETURN, file_code_writer) == WC_UNSUPPORTED_CMD);
    mu_assert("vm_write_command did not return WC_UNSUPPORTED_CMD when given a command of type C_CALL",
        vm_write_command("call mult 2", C_CALL, file_code_writer) == WC_UNSUPPORTED_CMD);

    vm_code_writer_close(file_code_writer);


    // Test vm_write_initial()
    char *init = vm_write_initial("a/b/c/d.vm");

    mu_assert("vm_write_initial doesn't return the correct bootstrapping assembly commands",
        !strcmp(init,
            "// Generated by Nand2Tetris VM translator (written by Jesse Evers)\n"
            "// File: a/b/c/d.vm\n\n"
            "// Initialize stack pointer\n"
            "@256\n"
            "D=A\n"
            "@SP\n"
            "M=D\n"
            "(Sys.init)\n"
            "@Main.main\n"
            "0;JMP\n"
            "// Begin user-defined program\n"));

    free(init);


    // Test vm_write_arithmetic()
    char *translate_add = vm_write_arithmetic("add");
    char *translate_eq = vm_write_arithmetic("eq");
    mu_assert("vm_write_arithmetic doesn't jump to the \"add\" command correctly",
        !strcmp(translate_add,
            "@POST_ARITH_CALL_1\n"
            "D=A\n"
            "@R13\n"
            "M=D\n"
            "@__ADD_OP\n"
            "0;JMP\n"
            "(POST_ARITH_CALL_1)\n"));
    mu_assert("vm_write_arithmetic doesn't jump to the \"eq\" command correctly",
        !strcmp(translate_eq,
            "@POST_ARITH_CALL_2\n"
            "D=A\n"
            "@R13\n"
            "M=D\n"
            "@__EQ_OP\n"
            "0;JMP\n"
            "(POST_ARITH_CALL_2)\n"));

    reinit_char(&translate_add);
    reinit_char(&translate_eq);


    // Test vm_write_push_pop()
    char *translate_push_const = vm_write_push_pop(CONSTANT, 23, C_PUSH, "Test");
    char *translate_push_arg = vm_write_push_pop(ARG, 8, C_PUSH, "Test");
    char *translate_pop_local = vm_write_push_pop(LCL, 2, C_POP, "Test");
    char *translate_push_temp = vm_write_push_pop(TEMP, 6, C_PUSH, "Test");
    char *translate_pop_pointer = vm_write_push_pop(POINTER, 1, C_POP, "Test");
    char *translate_push_static = vm_write_push_pop(STATIC, 2, C_PUSH, "Test");
    char *translate_pop_static = vm_write_push_pop(STATIC, 4, C_POP, "Test");

    mu_assert("vm_write_push_pop doesn't translate `push constant 23` correctly",
        !strcmp(translate_push_const,
            "@SP\n"
            "M=M+1\n"
            "@23\n"
            "D=A\n"
            "@SP\n"
            "A=M-1\n"
            "M=D\n"));
    mu_assert("vm_write_push_pop doesn't translate `push argument 8` correctly",
        !strcmp(translate_push_arg,
            "@ARG\n"
            "D=M\n"
            "@8\n"
            "A=D+A\n"
            "D=M\n"
            "@SP\n"
            "M=M+1\n"
            "A=M-1\n"
            "M=D\n"));
    mu_assert("vm_write_push_pop doesn't translate `pop local 2` correctly",
        !strcmp(translate_pop_local,
            "@LCL\n"
            "D=M\n"
            "@2\n"
            "D=D+A\n"
            "@R15\n"
            "M=D\n"
            "@SP\n"
            "AM=M-1\n"
            "D=M\n"
            "@R15\n"
            "A=M\n"
            "M=D\n"));
    mu_assert("vm_write_push_pop doesn't translate `push temp 6` correctly",
        !strcmp(translate_push_temp,
            "@11\n"
            "D=M\n"
            "@SP\n"
            "M=M+1\n"
            "A=M-1\n"
            "M=D\n"));
    mu_assert("vm_write_push_pop doesn't translate `pop pointer 1` correctly",
        !strcmp(translate_pop_pointer,
            "@SP\n"
            "AM=M-1\n"
            "D=M\n"
            "@4\n"
            "M=D\n"));
    mu_assert("vm_write_push_pop doesn't translate `push static 2` correctly",
        !strcmp(translate_push_static,
            "@Test.2\n"
            "D=M\n"
            "@SP\n"
            "M=M+1\n"
            "A=M-1\n"
            "M=D\n"));
    mu_assert("vm_write_push_pop doesn't translate `pop static 4` correctly",
        !strcmp(translate_pop_static,
            "@SP\n"
            "AM=M-1\n"
            "D=M\n"
            "@Test.4\n"
            "M=D\n"));

    reinit_char(&translate_push_const);
    reinit_char(&translate_push_arg);
    reinit_char(&translate_pop_local);
    reinit_char(&translate_push_temp);
    reinit_char(&translate_pop_pointer);
    reinit_char(&translate_push_static);
    reinit_char(&translate_pop_static);


    // Test vm_write_label()
    char *valid_label = vm_write_label("test", "abc_123.foo:bar");
    char *invalid_label = vm_write_label("test", "going-wild?");

    mu_assert("vm_write_label does not successfully define a valid label",
        !strcmp(valid_label, "(test:abc_123.foo:bar)\n"));
    mu_assert("vm_write_label translates a label with invalid characters", invalid_label == NULL);

    reinit_char(&valid_label);
    reinit_char(&invalid_label);

    return 0;
}

static char *test_util() {
    // Test is_directory()
    mu_assert("is_directory says that ./src/test/ is not a directory", is_directory("./src/test/"));
    mu_assert("is_directory says that ./src/test/Test.vm/ is a directory", !is_directory("./src/test/Test.vm"));


    // Test path_parts_cmp()
    path_parts pp1 = {"/a/b/c/", "d.e"};
    path_parts pp2 = {"./asdf/qwer", NULL};
    path_parts pp3 = {NULL, "file.ext"};
    path_parts pp4 = {NULL, NULL};

    mu_assert("path_parts_cmp incorrectly asserts that a path_parts struct with no NULL fields is not equal to itself",
        path_parts_cmp(&pp1, &pp1));
    mu_assert("path_parts_cmp incorrectly asserts that a path_parts struct with its dirname field set to NULL is not equal to itself",
        path_parts_cmp(&pp2, &pp2));
    mu_assert("path_parts_cmp incorrectly asserts that a path_parts struct with its basename field set to NULL is not equal to itself",
        path_parts_cmp(&pp3, &pp3));
    mu_assert("path_parts_cmp incorrectly asserts that two path_parts structs with opposite fields set to NULL are equal",
        path_parts_cmp(&pp2, &pp3) == 0);
    mu_assert("path_parts_cmp incorrectly asserts that a path_parts struct with both fields set to NULL does not equal itself",
        path_parts_cmp(&pp4, &pp4));


    // Test path_parts_split()
    path_parts *split_path_dir, *split_path_dir_trailing_slash, *split_path_file, *split_path_relative, *split_path_single_segment;
    split_path_dir = calloc(1, sizeof(path_parts));
    split_path_dir_trailing_slash = calloc(1, sizeof(path_parts));
    split_path_file = calloc(1, sizeof(path_parts));
    split_path_relative = calloc(1, sizeof(path_parts));
    split_path_single_segment = calloc(1, sizeof(path_parts));

    split_path_dir->basename = calloc(strlen("./src/test"), sizeof(char));
    split_path_dir->dirname = calloc(strlen("./src/test"), sizeof(char));
    split_path_dir_trailing_slash->basename = calloc(strlen("./src/test/"), sizeof(char));
    split_path_dir_trailing_slash->dirname = calloc(strlen("./src/test/"), sizeof(char));
    split_path_file->basename = calloc(strlen("../Makefile"), sizeof(char));
    split_path_file->dirname = calloc(strlen("../Makefile"), sizeof(char));
    split_path_relative->basename = calloc(strlen("../../"), sizeof(char));
    split_path_relative->dirname = calloc(strlen("../../"), sizeof(char));
    split_path_single_segment->basename = calloc(strlen("src/"), sizeof(char));
    split_path_single_segment->dirname = calloc(strlen("src/"), sizeof(char));

    path_parts_split(split_path_dir, "./src/test");
    path_parts_split(split_path_dir_trailing_slash, "./src/test/");
    path_parts_split(split_path_file, "../Makefile");
    path_parts_split(split_path_relative, "../../");
    path_parts_split(split_path_single_segment, "src/");

    mu_assert("split_path is incorrect when given a path ending in a directory without a trailing slash",
        path_parts_cmp(split_path_dir, &((path_parts){"./src/", "test"})));
    mu_assert("split_path is incorrect when given a path ending in a directory with a trailing slash",
        path_parts_cmp(split_path_dir_trailing_slash, &((path_parts){"./src/", "test"})));
    mu_assert("split_path is incorrect when given a path ending in a file",
        path_parts_cmp(split_path_file, &((path_parts){"../", "Makefile"})));
    mu_assert("split_path is incorrect when given a path only containing relative directories, ending with a trailing slash",
        path_parts_cmp(split_path_relative, &((path_parts){"../", ".."})));
    mu_assert("split_path is incorrect when given a path that only consists of a single file/directory",
        path_parts_cmp(split_path_single_segment, &((path_parts){"", "src"})));


    // Test path_parts_delete()
    // I'm really just testing this by using valgrind to ensure there are no memory leaks
    path_parts_delete(&split_path_dir);
    path_parts_delete(&split_path_dir_trailing_slash);
    path_parts_delete(&split_path_file);
    path_parts_delete(&split_path_relative);
    path_parts_delete(&split_path_single_segment);


    // Test remove_fext()
    char *remove_fext_no_fext = remove_fext("Makefile");
    char *remove_fext_single_char_fext = remove_fext("test.c");
    char *remove_fext_long_fext = remove_fext("foo.qwertyuiop");

    mu_assert("remove_fext incorrectly processes filenames with no file extension", !strcmp(remove_fext_no_fext, "Makefile"));
    mu_assert("remove_fext incorrectly processes filenames with a single-character file extension",
        !strcmp(remove_fext_single_char_fext, "test"));
    mu_assert("remove_fext incorrectly processes filenames with long file extensions",
        !strcmp(remove_fext_long_fext, "foo"));

    reinit_char(&remove_fext_no_fext);
    reinit_char(&remove_fext_single_char_fext);
    reinit_char(&remove_fext_long_fext);


    // Test toupper_str()
    char *all_lower = "abcd";
    char *mixed_case = "4Daf!!";
    char *lower_upper = calloc(strlen("abcd") + 1, sizeof(char));
    char *mixed_upper = calloc(strlen("4Daf!!") + 1, sizeof(char));
    toupper_str(lower_upper, all_lower);
    toupper_str(mixed_upper, mixed_case);

    mu_assert("toupper_str doesn't properly convert \"abcd\" to uppercase", !strcmp(lower_upper, "ABCD"));
    mu_assert("toupper_str doesn't properly convert \"4Daf!!\" to uppercase", !strcmp(mixed_upper, "4DAF!!"));

    reinit_char(&lower_upper);
    reinit_char(&mixed_upper);


    // Test vm_strcmp()
    char *s1 = "abc";
    char *s2 = "def";

    mu_assert("vm_strcmp asserts that a non-NULL string is not equal to itself", !vm_strcmp(s1, s1));
    mu_assert("vm_strcmp asserts that a non-NULL string is equal to a different non-NULL string", !vm_strcmp(s1, s2) == 0);
    mu_assert("vm_strcmp asserts that a non-NULL string is equal to a NULL string", !vm_strcmp(s2, NULL) == 0);
    mu_assert("vm_strcmp asserts that a NULL string is not equal to NULL", !vm_strcmp(NULL, NULL));


    // Test fmt_str_new()
    const char *str1 = "This string needs to %s formatted";
    const char *str2 = NULL;
    fmt_str *fs1 = fmt_str_new(str1, 2);
    fmt_str *fs2 = fmt_str_new(str2, 7);
    fmt_str *fs3 = NULL;

    mu_assert("fmt_str_new did not properly initialize a fmt_str when given valid, non-NULL input data",
        fs1 != NULL && !strcmp(fs1->str, str1) && fs1->fmt_len == 2);
    mu_assert("fmt_str_new did not properly initialize a fmt_str when given a NULL format string",
        fs2 != NULL && fs2->str == NULL && fs2->fmt_len == 0);


    // Test fmt_str_len()
    mu_assert("fmt_str_len did not correctly compute the length of a populated fmt_str", fmt_str_len(fs1) == 31);
    mu_assert("fmt_str_len did not correctly compute the length of a fmt_str with a NULL format string", fmt_str_len(fs2) == -1);
    mu_assert("fmt_str_len did not correctly compute the length of a NULL fmt_str", fmt_str_len(fs3) == -1);


    // Test fmt_str_delete()
    fmt_str_delete(&fs1);
    fmt_str_delete(&fs2);
    fmt_str_delete(&fs3);

    mu_assert("fmt_str_delete did not free/set to NULL a populated fmt_str", fs1 == NULL);
    mu_assert("fmt_str_delete did not free/set to NULL a populated fmt_str", fs2 == NULL);
    mu_assert("fmt_str_delete did not leave a NULL fmt_str as NULL", fs3 == NULL);


    // Test reinit_char()
    char *to_reinit1 = calloc(4, sizeof(char));
    char *to_reinit2 = NULL;
    reinit_char(&to_reinit1);
    reinit_char(&to_reinit2);

    mu_assert("reinit_char does not successfully set a pointer to a non-NULL char* to NULL", to_reinit1 == NULL);
    mu_assert("reinit_char doesn't leave a pointer to a NULL char* as NULL", to_reinit2 == NULL);


    // Test num_digits()
    mu_assert("num_digits says that 0 doesn't have 1 digit", num_digits(0) == 1);
    mu_assert("num_digits says that 1 doesn't have 1 digit", num_digits(1) == 1);
    mu_assert("num_digits says that -1 doesn't have 2 digits", num_digits(-1) == 2);
    mu_assert("num_digits says that 3378 doesn't have 4 digits", num_digits(3378) == 4);
    mu_assert("num_digits says that -1234556 doesn't have 8 digits", num_digits(-1234556) == 8);

    return 0;
}

static char *all_tests() {
    mu_run_test(test_parser);
    mu_run_test(test_code_writer);
    mu_run_test(test_util);
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
