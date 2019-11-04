#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>


#ifndef _VM_CODE_WRITER_VARS
#define _VM_CODE_WRITER_VARS

const char* FOUT_EXT = ".asm";
const char DIR_SEP = '/';

#endif


/**
 * Opens the output file to which to write the assembly code that reuslts from translating the input
 * file(s). If @input_path is a path to a file, the output file will be named the same thing as the
 * input file, but with the file extension FOUT_EXT instead of .vm. If @input_path is a path to a
 * folder, the output file will be named the same thing as the input folder, but with the file
 * extension FOUT_EXT added to the end. In both cases, the output file will be created in the folder
 * that contains the input file/folder.
 *
 * @param  char*  input_path  The path to the file or folder being translated to assembly
 * @return FILE*              A file pointer to the output file
 */
FILE *VM_Code_Writer(char *input_path) {
    struct stat sb;
    // Check if input_path is a directory
    int is_dir = stat(input_path, &sb) == 0 && S_ISDIR(sb.st_mode);

    // Make sure we ignore any trailing DIR_SEP (e.g., the last character of the path
    // "/with/trailing/slash/") if it exists
    int input_path_len = (int)strlen(input_path);
    if (input_path[input_path_len - 1] == DIR_SEP) {
        input_path_len--;
    }

    char *path_copy = calloc(input_path_len + 1, sizeof(char));
    strncpy(path_copy, input_path, input_path_len);
    path_copy[input_path_len] = '\0';

    int last_slash_idx = input_path_len - 1;
    while(last_slash_idx >= 0 && input_path[last_slash_idx] != DIR_SEP) { last_slash_idx--; }

    int outfile_name_len = input_path_len - (last_slash_idx + 1);

    int period_idx = -1;
    // Only need to worry about handling the input file's file extension if the input path was a
    // path to a file
    if (!is_dir) {
        for (int i = strlen(path_copy) - 1; i >= 0; i--) {
            if (path_copy[i] == '.') {
                period_idx = i;
                break;
            }
        }

        // Remove the length of '.' + file extension from outfile_name_len, if the input file has
        // a file extension
        if (period_idx > -1) {
            outfile_name_len -= input_path_len - (period_idx + 1);
        }
    }

    // If there was no period in the input path, set period_idx to the length of the input path
    if (period_idx == -1) period_idx = input_path_len;

    // Get just the actual filename to write to -- e.g., outfile.asm
    char *outfile_name = calloc(outfile_name_len + (int)strlen(FOUT_EXT) + 1, sizeof(char));
    strncpy(outfile_name, path_copy + last_slash_idx + 1, period_idx - (last_slash_idx + 1));
    outfile_name[outfile_name_len] = '\0';
    strcat(outfile_name, FOUT_EXT);

    // Combine the path to the output file with the actual name of the output file. E.g.,
    // ./example/path/to/outfile.asm
    int full_output_path_len = (int)strlen(outfile_name) + (last_slash_idx + 1);
    char *output_path = calloc(full_output_path_len + 1, sizeof(char));
    strncpy(output_path, path_copy, last_slash_idx + 1);
    output_path[last_slash_idx + 1] = '\0';
    strcat(output_path, outfile_name);

    FILE *out = fopen(output_path, "a");

    free(path_copy);
    free(outfile_name);
    free(output_path);

    if (!out) {
        perror("Failed to open VM output file");
        return NULL;
    }

    return out;
}
