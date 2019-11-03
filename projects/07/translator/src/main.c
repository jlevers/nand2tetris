#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: ./translator path/to/prog.vm\n"
               "                 or path/to/project/dir/\n\n"
               "If passing a path to a directory, the directory should contain at least 1\n"
               ".vm file.");
        return EXIT_FAILURE;
    }

    printf("placeholder: %s\n", argv[0]);
}
