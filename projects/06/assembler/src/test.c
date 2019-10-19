#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "encoder.h"
#include "../hashtable/src/hash_table.h"
#include "minunit.h"
#include "parser.h"
#include "symboltable.h"

int tests_run = 0;

// Test encoder.c
static char *test_encoder() {
    // Test encode_dest()
    const char *dest_m = encode_dest("M\0");
    const char *dest_d = encode_dest("D\0");
    const char *dest_md = encode_dest("MD\0");
    const char *dest_dm = encode_dest("DM\0");
    const char *dest_a = encode_dest("A\0");
    const char *dest_am = encode_dest("AM\0");
    const char *dest_ma = encode_dest("MA\0");
    const char *dest_ad = encode_dest("AD\0");
    const char *dest_da = encode_dest("DA\0");
    const char *dest_amd = encode_dest("AMD\0");
    const char *dest_adm = encode_dest("ADM\0");
    const char *dest_mad = encode_dest("MAD\0");
    const char *dest_mda = encode_dest("MDA\0");
    const char *dest_dma = encode_dest("DMA\0");
    const char *dest_dam = encode_dest("DAM\0");
    mu_assert("NULL destination does not encode to 000", !strcmp(encode_dest(NULL), "000\0"));
    mu_assert("destination M does not encode to 001", !strcmp(dest_m, "001\0"));
    mu_assert("destination D does not encode to 010", !strcmp(dest_d, "010\0"));
    mu_assert("destination MD does not encode to 011", !strcmp(dest_md, "011\0"));
    mu_assert("destinations MD and DM do not encode to the same value",
        !strcmp(dest_md, dest_dm));
    mu_assert("destination A does not encode to 100", !strcmp(dest_a, "100\0"));
    mu_assert("destination AM does not encode to 101", !strcmp(dest_am, "101\0"));
    mu_assert("destinations AM and MA do not encode to the same value",
        !strcmp(dest_am, dest_ma));
    mu_assert("destination AD does not encode to 110", !strcmp(dest_ad, "110\0"));
    mu_assert("destinations AD and DA do not encode to the same value",
        !strcmp(dest_ad, dest_da));
    mu_assert("destination AMD does not encode to 111", !strcmp(dest_amd, "111\0"));
    mu_assert("destinations AMD, ADM, MAD, MDA, DMA, and DAM do not encode to the same value",
        !strcmp(dest_amd, dest_adm) &&
        !strcmp(dest_amd, dest_mad) &&
        !strcmp(dest_amd, dest_mda) &&
        !strcmp(dest_amd, dest_dma) &&
        !strcmp(dest_amd, dest_dam));

    // Test encode_comp()
    char *comp_0 = encode_comp("0\0");
    char *comp_1 = encode_comp("1\0");
    char *comp_neg_1 = encode_comp("-1\0");
    char *comp_d = encode_comp("D\0");
    char *comp_a = encode_comp("A\0");
    char *comp_m = encode_comp("M\0");
    char *comp_not_d = encode_comp("!D\0");
    char *comp_not_a = encode_comp("!A\0");
    char *comp_not_m = encode_comp("!M\0");
    char *comp_neg_d = encode_comp("-D\0");
    char *comp_neg_a = encode_comp("-A\0");
    char *comp_neg_m = encode_comp("-M\0");
    char *comp_d_plus_1 = encode_comp("D+1\0");
    char *comp_1_plus_d = encode_comp("1+D\0");
    char *comp_a_plus_1 = encode_comp("A+1\0");
    char *comp_1_plus_a = encode_comp("1+A\0");
    char *comp_m_plus_1 = encode_comp("M+1\0");
    char *comp_1_plus_m = encode_comp("1+M\0");
    char *comp_d_min_1 = encode_comp("D-1\0");
    char *comp_a_min_1 = encode_comp("A-1\0");
    char *comp_m_min_1 = encode_comp("M-1\0");
    char *comp_d_plus_a = encode_comp("D+A\0");
    char *comp_a_plus_d = encode_comp("A+D\0");
    char *comp_d_plus_m = encode_comp("D+M\0");
    char *comp_m_plus_d = encode_comp("M+D\0");
    char *comp_d_min_a = encode_comp("D-A\0");
    char *comp_d_min_m = encode_comp("D-M\0");
    char *comp_a_min_d = encode_comp("A-D\0");
    char *comp_m_min_d = encode_comp("M-D\0");
    char *comp_d_and_a = encode_comp("D&A\0");
    char *comp_a_and_d = encode_comp("A&D\0");
    char *comp_d_and_m = encode_comp("D&M\0");
    char *comp_m_and_d = encode_comp("M&D\0");
    char *comp_d_or_a = encode_comp("D|A\0");
    char *comp_a_or_d = encode_comp("A|D\0");
    char *comp_d_or_m = encode_comp("D|M\0");
    char *comp_m_or_d = encode_comp("M|D");
    mu_assert("computation 0 does not encode to 0101010", !strcmp(comp_0, "0101010\0"));
    mu_assert("computation 1 does not encode to 0111111", !strcmp(comp_1, "0111111\0"));
    mu_assert("computation -1 does not encode to 0111010", !strcmp(comp_neg_1, "0111010\0"));
    mu_assert("computation D does not encode to 0001100", !strcmp(comp_d, "0001100\0"));
    mu_assert("computation A does not encode to 0110000", !strcmp(comp_a, "0110000\0"));
    mu_assert("computation M does not encode to 1110000", !strcmp(comp_m, "1110000\0"));
    mu_assert("computation !D does not encode to 0001101", !strcmp(comp_not_d, "0001101\0"));
    mu_assert("computation !A does not encode to 0110001", !strcmp(comp_not_a, "0110001\0"));
    mu_assert("computation !M does not encode to 1110001", !strcmp(comp_not_m, "1110001\0"));
    mu_assert("computation -D does not encode to 0001111", !strcmp(comp_neg_d, "0001111\0"));
    mu_assert("computation -A does not encode to 0110011", !strcmp(comp_neg_a, "0110011\0"));
    mu_assert("computation -M does not encode to 1110011", !strcmp(comp_neg_m, "1110011\0"));
    mu_assert("computation D+1 does not encode to 0011111", !strcmp(comp_d_plus_1, "0011111\0"));
    mu_assert("computations D+1 and 1+D do not encode to the same thing",
        !strcmp(comp_d_plus_1, comp_1_plus_d));
    mu_assert("computation A+1 does not encode to 0110111", !strcmp(comp_a_plus_1, "0110111\0"));
    mu_assert("computations A+1 and 1+A do not encode to the same thing",
        !strcmp(comp_a_plus_1, comp_1_plus_a));
    mu_assert("computation M+1 does not encode to 1110111", !strcmp(comp_m_plus_1, "1110111\0"));
    mu_assert("computations M+1 and 1+M do not encode to the same thing",
        !strcmp(comp_m_plus_1, comp_1_plus_m));
    mu_assert("computation D-1 does not encode to 0001110", !strcmp(comp_d_min_1, "0001110\0"));
    mu_assert("computation A-1 does not encode to 0110010", !strcmp(comp_a_min_1, "0110010\0"));
    mu_assert("computation M-1 does not encode to 1110010", !strcmp(comp_m_min_1, "1110010\0"));
    mu_assert("computation D+A does not encode to 0000010", !strcmp(comp_d_plus_a, "0000010\0"));
    mu_assert("computations D+A and A+D do not encode to the same thing",
        !strcmp(comp_d_plus_a, comp_a_plus_d));
    mu_assert("computation D+M does not encode to 1000010", !strcmp(comp_d_plus_m, "1000010\0"));
    mu_assert("computations D+M and M+D do not encode to the same thing",
        !strcmp(comp_d_plus_m, comp_m_plus_d));
    mu_assert("computation D-A does not encode to 0010011", !strcmp(comp_d_min_a, "0010011\0"));
    mu_assert("computation D-M does not encode to 1010011", !strcmp(comp_d_min_m, "1010011\0"));
    mu_assert("computation A-D does not encode to 0000111", !strcmp(comp_a_min_d, "0000111\0"));
    mu_assert("computation M-D does not encode to 1000111", !strcmp(comp_m_min_d, "1000111\0"));
    mu_assert("computation D&A does not encode to 0000000", !strcmp(comp_d_and_a, "0000000\0"));
    mu_assert("computations D&A and A&D do not encode to the same thing",
        !strcmp(comp_d_and_a, comp_a_and_d));
    mu_assert("computation D&M does not encode to 1000000", !strcmp(comp_d_and_m, "1000000\0"));
    mu_assert("computations D&M and M&D do not encode to the same thing",
        !strcmp(comp_d_and_m, comp_m_and_d));
    mu_assert("computation D|A does not encode to 0010101", !strcmp(comp_d_or_a, "0010101\0"));
    mu_assert("computations D|A and A|D do not encode to the same thing",
        !strcmp(comp_d_or_a, comp_a_or_d));
    mu_assert("computation D|M does not encode to 1010101", !strcmp(comp_d_or_m, "1010101\0"));
    mu_assert("computations D|M and M|D do not encode to the same thing",
        !strcmp(comp_d_or_m, comp_m_or_d));
    free(comp_0);
    free(comp_1);
    free(comp_neg_1);
    free(comp_d);
    free(comp_a);
    free(comp_m);
    free(comp_not_d);
    free(comp_not_a);
    free(comp_not_m);
    free(comp_neg_d);
    free(comp_neg_a);
    free(comp_neg_m);
    free(comp_d_plus_1);
    free(comp_1_plus_d);
    free(comp_a_plus_1);
    free(comp_1_plus_a);
    free(comp_m_plus_1);
    free(comp_1_plus_m);
    free(comp_d_min_1);
    free(comp_a_min_1);
    free(comp_m_min_1);
    free(comp_d_plus_a);
    free(comp_a_plus_d);
    free(comp_d_plus_m);
    free(comp_m_plus_d);
    free(comp_d_min_a);
    free(comp_d_min_m);
    free(comp_a_min_d);
    free(comp_m_min_d);
    free(comp_d_and_a);
    free(comp_a_and_d);
    free(comp_d_and_m);
    free(comp_m_and_d);
    free(comp_d_or_a);
    free(comp_a_or_d);
    free(comp_d_or_m);
    free(comp_m_or_d);


    // Test encode_jump()
    const char *jmp_gt = encode_jump("JGT");
    const char *jmp_eq = encode_jump("JEQ");
    const char *jmp_ge = encode_jump("JGE");
    const char *jmp_lt = encode_jump("JLT");
    const char *jmp_ne = encode_jump("JNE");
    const char *jmp_le = encode_jump("JLE");
    const char *jmp_always = encode_jump("JMP");
    mu_assert("NULL jump code does not encode to 000", !strcmp(encode_jump(NULL), "000\0"));
    mu_assert("jump code JGT does not encode to 001", !strcmp(jmp_gt, "001\0"));
    mu_assert("jump code JEQ does not encode to 010", !strcmp(jmp_eq, "010\0"));
    mu_assert("jump code JGE does not encode to 011", !strcmp(jmp_ge, "011\0"));
    mu_assert("jump code JLT does not encode to 100", !strcmp(jmp_lt, "100\0"));
    mu_assert("jump code JNE does not encode to 101", !strcmp(jmp_ne, "101\0"));
    mu_assert("jump code JLE does not encode to 110", !strcmp(jmp_le, "110\0"));
    mu_assert("jump code JMP does not encode to 111", !strcmp(jmp_always, "111\0"));


    return 0;
}

// Test symbol_table.c
static char *test_symbol_table() {
    ht_hash_table *ht = constructor(10);
    mu_assert("symbol table is the wrong size", ht->size == 10);

    char *sp = ht_search(ht, "SP\0");
    char *lcl = ht_search(ht, "LCL\0");
    char *arg = ht_search(ht, "ARG\0");
    char *ths = ht_search(ht, "THIS\0");
    char *that = ht_search(ht, "THAT\0");
    char *r0 = ht_search(ht, "R0\0");
    char *r1 = ht_search(ht, "R1\0");
    char *r2 = ht_search(ht, "R2\0");
    char *r3 = ht_search(ht, "R3\0");
    char *r4 = ht_search(ht, "R4\0");
    char *r5 = ht_search(ht, "R5\0");
    char *r6 = ht_search(ht, "R6\0");
    char *r7 = ht_search(ht, "R7\0");
    char *r8 = ht_search(ht, "R8\0");
    char *r9 = ht_search(ht, "R9\0");
    char *r10 = ht_search(ht, "R10\0");
    char *r11 = ht_search(ht, "R11\0");
    char *r12 = ht_search(ht, "R12\0");
    char *r13 = ht_search(ht, "R13\0");
    char *r14 = ht_search(ht, "R14\0");
    char *r15 = ht_search(ht, "R15\0");
    char *screen = ht_search(ht, "SCREEN\0");
    char *kbd = ht_search(ht, "KBD\0");

    mu_assert("SP is not in initial symbol table", sp != NULL);
    mu_assert("symbol table has incorrect address for symbol SP", !strcmp(sp, "0000000000000000\0"));
    mu_assert("LCL is not in initial symbol table", lcl != NULL);
    mu_assert("symbol table has incorrect address for symbol LCL", !strcmp(lcl, "0000000000000001\0"));
    mu_assert("ARG is not in initial symbol table", arg != NULL);
    mu_assert("symbol table has incorrect address for symbol ARG", !strcmp(arg, "0000000000000010\0"));
    mu_assert("this is not in initial symbol table", ths != NULL);
    mu_assert("symbol table has incorrect address for symbol THIS", !strcmp(ths, "0000000000000011\0"));
    mu_assert("THAT is not in initial symbol table", that != NULL);
    mu_assert("symbol table has incorrect address for symbol THAT", !strcmp(that, "0000000000000100\0"));
    mu_assert("R0 is not in initial symbol table", r0 != NULL);
    mu_assert("symbol table has incorrect address for symbol R0", !strcmp(r0, "0000000000000000\0"));
    mu_assert("R1 is not in initial symbol table", r1 != NULL);
    mu_assert("symbol table has incorrect address for symbol R1", !strcmp(r1, "0000000000000001\0"));
    mu_assert("R2 is not in initial symbol table", r2 != NULL);
    mu_assert("symbol table has incorrect address for symbol R2", !strcmp(r2, "0000000000000010\0"));
    mu_assert("R3 is not in initial symbol table", r3 != NULL);
    mu_assert("symbol table has incorrect address for symbol R3", !strcmp(r3, "0000000000000011\0"));
    mu_assert("R4 is not in initial symbol table", r4 != NULL);
    mu_assert("symbol table has incorrect address for symbol R4", !strcmp(r4, "0000000000000100\0"));
    mu_assert("R5 is not in initial symbol table", r5 != NULL);
    mu_assert("symbol table has incorrect address for symbol R5", !strcmp(r5, "0000000000000101\0"));
    mu_assert("R6 is not in initial symbol table", r6 != NULL);
    mu_assert("symbol table has incorrect address for symbol R6", !strcmp(r6, "0000000000000110\0"));
    mu_assert("R7 is not in initial symbol table", r7 != NULL);
    mu_assert("symbol table has incorrect address for symbol R7", !strcmp(r7, "0000000000000111\0"));
    mu_assert("R8 is not in initial symbol table", r8 != NULL);
    mu_assert("symbol table has incorrect address for symbol R8", !strcmp(r8, "0000000000001000\0"));
    mu_assert("R9 is not in initial symbol table", r9 != NULL);
    mu_assert("symbol table has incorrect address for symbol R9", !strcmp(r9, "0000000000001001\0"));
    mu_assert("R10 is not in initial symbol table", r10 != NULL);
    mu_assert("symbol table has incorrect address for symbol R10", !strcmp(r10, "0000000000001010\0"));
    mu_assert("R11 is not in initial symbol table", r11 != NULL);
    mu_assert("symbol table has incorrect address for symbol R11", !strcmp(r11, "0000000000001011\0"));
    mu_assert("R12 is not in initial symbol table", r12 != NULL);
    mu_assert("symbol table has incorrect address for symbol R12", !strcmp(r12, "0000000000001100\0"));
    mu_assert("R13 is not in initial symbol table", r13 != NULL);
    mu_assert("symbol table has incorrect address for symbol R13", !strcmp(r13, "0000000000001101\0"));
    mu_assert("R14 is not in initial symbol table", r14 != NULL);
    mu_assert("symbol table has incorrect address for symbol R14", !strcmp(r14, "0000000000001110\0"));
    mu_assert("R15 is not in initial symbol table", r15 != NULL);
    mu_assert("symbol table has incorrect address for symbol R15", !strcmp(r15, "0000000000001111\0"));
    mu_assert("SCREEN is not in initial symbol table", screen != NULL);
    mu_assert("symbol table has incorrect address for symbol SCREEN", !strcmp(screen, "0100000000000000\0"));
    mu_assert("KBD is not in initial symbol table", kbd != NULL);
    mu_assert("symbol table has incorrect address for symbol KBD", !strcmp(kbd, "0110000000000000\0"));

    ht_delete(ht);

    return 0;
}

// Shamelessly taken from https://stackoverflow.com/a/12502754/3696964
int same_file(int fd1, int fd2) {
    struct stat stat1, stat2;
    if (fstat(fd1, &stat1) < 0) return -1;
    if (fstat(fd2, &stat2) < 0) return -1;
    return (stat1.st_dev == stat2.st_dev) && (stat1.st_ino == stat2.st_ino);
}

void reinit_char(char **to_process) {
    free(*to_process);
    *to_process = NULL;
}

static char *test_parser() {
    // Test init()
    const char *fin_name = "./src/test/Test.asm";
    const char *fout_name = "./src/test/Test.hack";
    io files = init(fin_name);
    mu_assert("init does not correctly open input file",
        same_file(fileno(files.in), open(fin_name, 'r')));
    mu_assert("init does not correctly open output file",
        same_file(fileno(files.out), open(fout_name, 'r')));

    // Test advance()
    char *line = advance(files.in);
    mu_assert("advance did not skip full-line comment and/or empty line",
        !strcmp(line, "(INFINITE_LOOP)\0"));
    reinit_char(&line);
    line = advance(files.in);
    mu_assert("advance did not properly ignore inline comment", !strcmp(line, "@INFINITE_LOOP\0"));
    reinit_char(&line);
    line = advance(files.in);
    reinit_char(&line);
    line = advance(files.in);
    mu_assert("advance did not return NULL at EOF", line == NULL);
    reinit_char(&line);

    fclose(files.in);
    fclose(files.out);

    // Test command_type()
    mu_assert("command_type did not recognize an A command", command_type("@0") == A_COMMAND);
    mu_assert("command_type did not recognize a symbolic A command",
        command_type("@sym") == A_COMMAND);
    mu_assert("command_type did not recognize a C command", command_type("0;JMP") == C_COMMAND);
    mu_assert("command_type did not recognize an L command", command_type("(LABEL)") == L_COMMAND);


    // Test parse_symbol()
    char *parsed_at_sym = parse_symbol(A_COMMAND, "@abc");
    char *parsed_label_sym = parse_symbol(L_COMMAND, "(TEST_LABEL)");
    mu_assert("parse_symbol did not correctly parse an @-prefixed symbol", !strcmp(parsed_at_sym, "abc\0"));
    mu_assert("parse_symbol did not correctly parse a label", !strcmp(parsed_label_sym, "TEST_LABEL\0"));
    reinit_char(&parsed_at_sym);
    reinit_char(&parsed_label_sym);


    // Test parse_dest()
    char *parsed_empty_dest = parse_dest("D;JGT");
    char *parsed_no_jump_dest = parse_dest("M=D");
    char *parsed_two_letter_dest = parse_dest("MA=D;JLE");
    char *parsed_three_letter_dest = parse_dest("AMD=M;JMP");
    mu_assert("parse_dest did not return NULL given a command with no destination",
        parsed_empty_dest == NULL);
    mu_assert("parse_dest failed to parse correct destination in command with no jump statement",
        !strcmp(parsed_no_jump_dest, "M\0"));
    mu_assert("parse_dest falied to parse two-letter destination",
        !strcmp(parsed_two_letter_dest, "MA\0"));
    mu_assert("parse_dest failed to parse three-letter destination",
        !strcmp(parsed_three_letter_dest, "AMD\0"));
    reinit_char(&parsed_empty_dest);
    reinit_char(&parsed_no_jump_dest);
    reinit_char(&parsed_two_letter_dest);
    reinit_char(&parsed_three_letter_dest);


    // Test parse_comp()
    char *parsed_comp_solo = parse_comp("D+1");
    char *parsed_comp_jump = parse_comp("M|A;JGT");
    char *parsed_comp_assign = parse_comp("M=A-1");
    char *parsed_comp_all = parse_comp("D=!M;JNE");
    char *parsed_comp_one_letter = parse_comp("MD=0");
    mu_assert("parse_comp did not correctly parse a standalone computation",
        !strcmp(parsed_comp_solo, "D+1\0"));
    mu_assert("parse_comp did not correctly parse a computation with a jump afterwards",
        !strcmp(parsed_comp_jump, "M|A\0"));
    mu_assert("parse_comp did not correctly parse a computation with assignment",
        !strcmp(parsed_comp_assign, "A-1\0"));
    mu_assert("parse_comp did not correctly parse a computation with assignment and a jump",
        !strcmp(parsed_comp_all, "!M\0"));
    mu_assert("parse_comp did not correctly parse a single-letter computation",
        !strcmp(parsed_comp_one_letter, "0\0"));
    reinit_char(&parsed_comp_solo);
    reinit_char(&parsed_comp_jump);
    reinit_char(&parsed_comp_assign);
    reinit_char(&parsed_comp_all);
    reinit_char(&parsed_comp_one_letter);


    // Test parse_jump()
    char *parsed_no_jump = parse_jump("M=AD");
    char *parsed_no_jump_sep = parse_jump("M=AD;");
    char *parsed_jump_comp = parse_jump("D&M;JLE");
    char *parsed_jump_all = parse_jump("M=A+1;JMP");
    mu_assert("parse_jump did not return NULL when given a command with no jump statement",
        parsed_no_jump == NULL);
    mu_assert("parse_jump did not return NULL when given a command with no jump statement, but "
        "with a jump command separator",parsed_no_jump_sep == NULL);
    mu_assert("parse_jump did not correctly parse a jump statement after a computation",
        !strcmp(parsed_jump_comp, "JLE\0"));
    mu_assert("parse_jump did not correctly parse a jump statement after assignment/computation",
        !strcmp(parsed_jump_all, "JMP\0"));
    reinit_char(&parsed_no_jump);
    reinit_char(&parsed_no_jump_sep);
    reinit_char(&parsed_jump_comp);
    reinit_char(&parsed_jump_all);


    // Test parse_to_binary()
    char *parse_0_2 = parse_to_binary(0);
    char *parse_1_2 = parse_to_binary(1);
    char *parse_2_2 = parse_to_binary(2);
    char *parse_3_2 = parse_to_binary(3);
    char *parse_4_2 = parse_to_binary(4);
    char *parse_7_2 = parse_to_binary(7);
    char *parse_8_2 = parse_to_binary(8);
    char *parse_83_2 = parse_to_binary(83);
    char *parse_2297_2 = parse_to_binary(2297);
    char *parse_10001_2 = parse_to_binary(10001);
    char *parse_54321_2 = parse_to_binary(54321);
    char *parse_65535_2 = parse_to_binary(65535);
    mu_assert("parse_to_binary outputted a string of the wrong length", strlen(parse_8_2) == 16);
    mu_assert("parse_to_binary did not correctly convert 0 to binary",
        !strcmp(parse_0_2, "0000000000000000\0"));
    mu_assert("parse_to_binary did not correctly convert 1 to binary",
        !strcmp(parse_1_2, "0000000000000001\0"));
    mu_assert("parse_to_binary did not correctly convert 2 to binary",
        !strcmp(parse_2_2, "0000000000000010\0"));
    mu_assert("parse_to_binary did not correctly convert 3 to binary",
        !strcmp(parse_3_2, "0000000000000011\0"));
    mu_assert("parse_to_binary did not correctly convert 4 to binary",
        !strcmp(parse_4_2, "0000000000000100\0"));
    mu_assert("parse_to_binary did not correctly convert 7 to binary",
        !strcmp(parse_7_2, "0000000000000111\0"));
    mu_assert("parse_to_binary did not correctly convert 8 to binary",
        !strcmp(parse_8_2, "0000000000001000\0"));
    mu_assert("parse_to_binary did not correctly convert 83 to binary",
        !strcmp(parse_83_2, "0000000001010011\0"));
    mu_assert("parse_to_binary did not correctly convert 2297 to binary",
        !strcmp(parse_2297_2, "0000100011111001\0"));
    mu_assert("parse_to_binary did not correctly convert 10001 to binary",
        !strcmp(parse_10001_2, "0010011100010001\0"));
    mu_assert("parse_to_binary did not correctly convert 54321 to binary",
        !strcmp(parse_54321_2, "1101010000110001\0"));
    mu_assert("parse_to_binary did not correctly convert 65535 to binary",
        !strcmp(parse_65535_2, "1111111111111111\0"));
    reinit_char(&parse_0_2);
    reinit_char(&parse_1_2);
    reinit_char(&parse_2_2);
    reinit_char(&parse_3_2);
    reinit_char(&parse_4_2);
    reinit_char(&parse_7_2);
    reinit_char(&parse_8_2);
    reinit_char(&parse_83_2);
    reinit_char(&parse_2297_2);
    reinit_char(&parse_10001_2);
    reinit_char(&parse_54321_2);
    reinit_char(&parse_65535_2);


    // Test first_pass()
    ht_hash_table *ht = constructor(2 * (19 / 3));
    const char *in = "../rect/Rect.asm";
    io fp_files = init(in);

    first_pass(fp_files.in, ht);

    char *loop = ht_search(ht, "LOOP\0");
    char *infinite_loop = ht_search(ht, "INFINITE_LOOP\0");
    mu_assert("first_pass failed to insert at least one label into the symbol table",
        (loop != NULL && !strcmp(loop, "0000000000001010\0")) &&
        (infinite_loop != NULL && !strcmp(infinite_loop, "0000000000010111\0")));
    reinit_char(&loop);
    reinit_char(&infinite_loop);


    // Test second_pass()
    fseek(fp_files.in, 0, SEEK_SET);
    second_pass(fp_files.in, fp_files.out, ht);

    char *counter = ht_search(ht, "counter\0");
    char *address = ht_search(ht, "address\0");
    mu_assert("second pass failed to insert at least one symbol into the symbol table",
        !strcmp(counter, "0000000000010000\0") && !strcmp(address, "0000000000010001\0"));

    fclose(fp_files.in);
    fclose(fp_files.out);
    ht_delete(ht);

    return 0;
}

static char *all_tests() {
    mu_run_test(test_encoder);
    mu_run_test(test_symbol_table);
    mu_run_test(test_parser);
    return 0;
}

int main(int argc, char **argv) {
    setbuf(stdout, NULL);
    char *result = all_tests();
    if (result != 0) {
        printf("%s\n", result);
    } else {
        printf("ALL TESTS PASSED\n");
    }
    printf("Tests run: %d\n", tests_run);

    return result != 0;
}
