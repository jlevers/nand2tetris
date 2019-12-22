#include <stdlib.h>
#include <stdio.h>

#include "code_writer.h"
#include "parser.h"
#include "tinydir.h"
#include "util.h"


void process_file(char *in_path, code_writer *cw) {
    path_parts *in_path_parts = calloc(1, sizeof(path_parts));
    in_path_parts->dirname = calloc(strlen(in_path) + 1, sizeof(char));
    in_path_parts->basename = calloc(strlen(in_path) + 1, sizeof(char));
    path_parts_split(in_path_parts, in_path);
    char *infile_name_noext = remove_fext(in_path_parts->basename);
    char *line = NULL;
    FILE *infile = fopen(in_path, "r");

    int infile_name_noext_len = strlen(infile_name_noext);
    strncpy(cw->in_name, infile_name_noext, infile_name_noext_len);
    cw->in_name[infile_name_noext_len] = '\0';

    while ((line = vm_advance(infile)) != NULL) {
        vm_write_command(line, vm_command_type(line), cw);
        reinit_char(&line);
    }

    fclose(infile);
    reinit_char(&infile_name_noext);
    reinit_char(&(cw->in_name));
    path_parts_delete(&in_path_parts);
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

    code_writer *cw = calloc(1, sizeof(code_writer));
    cw->in_name = calloc(strlen(argv[1]), sizeof(char));
    cw->out = NULL;
    VM_Code_Writer(argv[1], cw);
    FILE *infile = NULL;


    if (is_directory(argv[1])) {
        tinydir_dir dir;
        tinydir_open(&dir, argv[1]);

        while (dir.has_next) {
            tinydir_file file;
            tinydir_readfile(&dir, &file);
            if (file.is_reg && !strcmp(file.extension, VM_FILE_EXT)) {
                infile = fopen(file.path, "r");
                process_file(file.path, cw);
                fclose(infile);
                infile = NULL;
            }

            tinydir_next(&dir);
        }
    } else {
        process_file(argv[1], cw);
    }

    vm_code_writer_close(cw);

    return 0;
}