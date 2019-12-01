#include <sys/stat.h>

/**
 * Checks if the given path is a directory.
 * 
 * @param  char*  path  The path to check
 * @return int          1 if the path is a directory, 0 otherwise
 */
int is_directory(const char *path) {
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISDIR(path_stat.st_mode);
}
