/*
 * Parser for the nand2tetris assembler.
 * @author Jesse Evers
 * @email jesse27999@gmail.com
 * @created 5/22/19
 */

#include "parser.h"

FILE *in;
char *command;

FILE* init(const char* filename) {
    FILE *fp = fopen(filename, "r");

    if (!fp) {
        perror("Failed to open file");
        return NULL;
    }

    return fp;
}

void process_file() {
    command_t cmd_type;
    char* destination, computation, jump_to;
    while (has_more_commands()) {
        advance();
        cmd_type = command_type();
        destination = dest();
        computation = comp();
        jump_to = jump();
    }
    free(destination);
    free(computation);
    free(jump_to);
}

int has_more_commands() {
    return ferror(in) != EOF;
}

void advance() {
    char c;
    size_t line_size = 0;
    command = calloc(line_size, sizeof(char));
    while ((c = fgetc(assembly)) != EOF) {
        if (c == '\n') {
            line_size = 0;
            command = realloc(command, 0);
        } else {
            command = realloc(command, sizeof(char) * ++line_size);
            command[line_size - 1] = c;
        }
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: assembler path/to/prog.asm");
        return EXIT_FAILURE;
    }

    in = init(argv[1]);
    process_file();

    free(in);
    free(command);
    return 0;
}
