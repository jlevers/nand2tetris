#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

/**
 * Checks if the given path is a directory.
 * 
 * @param path the path to check
 * @return     1 if the path is a directory, 0 otherwise
 */
int is_directory(const char *path) {
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISDIR(path_stat.st_mode);
}


/**
 * Converts the given string to uppercase.
 *
 * @param dest the string to write the uppercase string into
 * @param str  the string to convert
 * @return     the string in uppercase
 */
char *toupper_str(char *dest, const char *str) {
    int str_len = strlen(str);

    int i;
    for (i = 0; i < str_len; i++) {
        dest[i] = toupper(str[i]);
    }
    dest[i] = '\0';

    return dest;
}


/**
 * Counts the number of digits in the given integer.
 *
 * @param num the number whose number of digits is to be counted
 * @return    the number of digits in num
 */
int num_digits(const int num) {
    int copy = num;
    int count = 0;

    if (copy <= 0) {
        count++;
        copy = -1 * copy;
    }

    while (copy > 0) {
        count++;
        copy /= 10;
    }
    return count;
}
