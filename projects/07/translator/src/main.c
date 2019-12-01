#include <stdlib.h>
#include <stdio.h>

#include "code_writer.h"
#include "parser.h"
#include "tinydir.h"
#include "util.h"


void process_file(FILE* in, FILE* out) {
    char *line = NULL;
    while ((line = vm_advance(in)) != NULL) {
        vm_write_command(line, vm_command_type(line), out);
    }
}


int main(int argc, char **argv) {
    const char *VM_FILE_EXT = "vm";

    if (argc != 2) {
        printf("Usage: ./translator path/to/prog.vm\n"
               "                 or path/to/project/dir/\n\n"
               "If passing a path to a directory, the directory should contain at least 1\n"
               ".vm file.");
        return EXIT_FAILURE;
    }

    FILE *outfile = VM_Code_Writer(argv[1]);
    FILE *infile = NULL;


    if (is_directory(argv[1])) {
        tinydir_dir dir;
        tinydir_open(&dir, argv[1]);

        while (dir.has_next) {
            tinydir_file file;
            tinydir_readfile(&dir, &file);
            if (file.is_reg && !strcmp(file.extension, VM_FILE_EXT)) {
                infile = fopen(file.path, "r");
                process_file(infile, outfile);
                fclose(infile);
                infile = NULL;
            }

            tinydir_next(&dir);
        }
    } else {
        infile = fopen(argv[1], "r");
        process_file(infile, outfile);
        fclose(infile);
        infile = NULL;
    }

    vm_code_writer_close(outfile);

    fclose(outfile);
    outfile = NULL;

    return 0;
}