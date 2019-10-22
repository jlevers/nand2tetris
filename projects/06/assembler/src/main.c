/*
 * Main controller for the nand2tetris assembler.
 * @author Jesse Evers
 * @email jesse27999@gmail.com
 */

#include <stdio.h>
#include <stdlib.h>

#include "parser.h"
#include "symboltable.h"
#include "../hashtable/src/hash_table.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: ./assembler path/to/prog.asm");
        return EXIT_FAILURE;
    }

    io files = init(argv[1]);
    FILE *in = files.in;
    FILE *out = files.out;

    // Count number of lines in file
    int line_count = 0;
    for (char c = getc(in); c != EOF; c = getc(in)) {
        if (c == '\n') line_count++;
    }
    fseek(in, 0, SEEK_SET);

    // 2/3rds the number of lines in the file is the maximum number of variables and labels possible
    // given the assembly syntax we're using
    ht_hash_table *ht = constructor(2 * (line_count / 3));

    first_pass(in, ht);
    fseek(in, 0, SEEK_SET);
    second_pass(in, out, ht);

    fclose(in);
    fclose(out);
    ht_delete(ht);
    return 0;
}
