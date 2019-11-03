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
    FILE *parser_in = Parser(fin_name);
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
    mu_assert("vm_command_type did not return C_PUSH when given command `push`",
        vm_command_type("push") == C_PUSH);
    mu_assert("vm_command_type did not return C_POP when given command `pop`",
        vm_command_type("pop") == C_POP);
    mu_assert("vm_command_type did not return C_LABEL when given command `label`",
        vm_command_type("label") == C_LABEL);
    mu_assert("vm_command_type did not return C_GOTO when given command `goto`",
        vm_command_type("if-goto") == C_GOTO);
    mu_assert("vm_command_type did not return C_IF when given command `if-goto`",
        vm_command_type("if-goto") == C_IF);
    mu_assert("vm_command_type did not return C_FUNCTION when given command `function`",
        vm_command_type("if-goto") == C_FUNCTION);
    mu_assert("vm_command_type did not return C_RETURN when given command `return`",
        vm_command_type("if-goto") == C_RETURN);
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

    return 0;
}

static char* test_code_writer() {
    return 0;
}

static char *all_tests() {
    mu_run_test(test_parser);
    mu_run_test(test_code_writer);
    return 0;
}

int main() {
    char *result = all_tests();
    if (result != 0) {
        printf("%s\n", result);
    } else {
        printf("ALL TESTS PASSED\n");
    }
    printf("Tests run: %d\n", tests_run);

    return result != 0;
}
