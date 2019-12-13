#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "util.h"

#ifndef _VM_UTIL_VARS
#define _VM_UTIL_VARS

const char DIR_SEP = '/';

#endif


/************************/
/*** Filesystem tools ***/
/************************/

/**
 * Checks if the given path is a directory.
 *
 * @param path the path to check
 * @return     1 if the path is a directory, 0 otherwise
 */
int is_directory(char *path) {
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISDIR(path_stat.st_mode);
}


/**
 * Splits a path into a basename and a dirname. The basename is the last location in the path (if the given path
 * had a trailing slash, it is not included in the basename). The dirname is the rest of the path. This data is
 * stored in a path_parts struct (see util.h).
 *
 * E.g., for the input "/asdf/foo/bar/baz.c", the basename is "baz.c", and the outer path is "/asdf/foo/bar/".
 *
 * @param p    the path_parts struct to store data in
 * @param path the path to parse
 */
void path_parts_split(path_parts *p, char *path) {
    // Make sure we ignore any trailing DIR_SEP (e.g., the last character of the path
    // "/with/trailing/slash/") if it exists
    int path_len = (int)strlen(path);
    if (path[path_len - 1] == DIR_SEP) {
        path_len--;
    }

    int last_slash_idx = path_len - 1;
    while(last_slash_idx >= 0 && path[last_slash_idx] != DIR_SEP) {
        last_slash_idx--;
    }

    int basename_len = path_len - (last_slash_idx + 1);
    strncpy(p->basename, path + (path_len - basename_len), basename_len);
    p->basename[basename_len] = '\0';

    int dirname_len = path_len - basename_len;
    strncpy(p->dirname, path, dirname_len);
    p->dirname[dirname_len] = '\0';
}


/**
 * Compares two path_parts structs.
 *
 * @param a a pointer to the first path_parts struct to compare
 * @param b a pointer to the second path_parts struct to compare
 * @return  1 if the structs are identical, 0 otherwise.
 */
int path_parts_cmp(path_parts *a, path_parts *b) {
    return vm_strcmp(a->basename, b->basename) == 0 && vm_strcmp(a->dirname, b->dirname) == 0;
}


/**
 * Deletes a path_parts struct.
 *
 * @param p a double pointer to the path_parts struct to delete
 */
void del_path_parts(path_parts **p) {
    if (*p != NULL) {
        if ((*p)->basename != NULL) {
            reinit_char(&((*p)->basename));
        }
        if ((*p)->dirname != NULL) {
            reinit_char(&((*p)->dirname));
        }
        free(*p);
        *p = NULL;
    }
}


/**
 * Removes the file extension from the given filename, if one exists.
 *
 * @param fname the filename whose extension is to be removed
 * @return      the filename without an extension
 */
char *remove_fext(char *fname) {
    int fname_len = strlen(fname);
    int fext_removed_len = fname_len;
    int period_idx = -1;

    for (int i = fname_len - 1; i >= 0; i--) {
        if (fname[i] == '.') {
            period_idx = i;
            break;
        }
    }

    // Subtract the length of '.' + file extension from fext_removed_len, if @fname has
    // a file extension
    if (period_idx > -1) {
        fext_removed_len -= fname_len - period_idx;
    }

    char *fext_removed = calloc(fext_removed_len + 1, sizeof(char));
    strncpy(fext_removed, fname, fext_removed_len);
    fext_removed[fext_removed_len] = '\0';

    return fext_removed;
}



/********************/
/*** String tools ***/
/********************/

/**
 * Converts the given string to uppercase.
 *
 * @param dest the string to write the uppercase string into
 * @param str  the string to convert
 * @return     the string in uppercase
 */
void toupper_str(char *dest, char *str) {
    int str_len = strlen(str);

    int i;
    for (i = 0; i < str_len; i++) {
        dest[i] = toupper(str[i]);
    }
    dest[i] = '\0';
}


/**
 * Compares two strings using strcmp, but also allows either string given to be NULL.
 *
 * @param a the first string to compare
 * @param b the second string to compare
 * @return  0 if the strings are equal, 1 if only one of them is NULL, and the value of strcmp(a, b) otherwise
 */
int vm_strcmp(char *a, char *b) {
    int diff;
    if (a == NULL && b == NULL) {
        diff = 0;
    } else if (a == NULL || b == NULL) {
        diff = 1;
    } else {
        diff = strcmp(a, b);
    }

    return diff;
}


/**
 * Frees a char pointer and sets it to NULL.
 *
 * @param to_process  A double pointer to a string
 */
void reinit_char(char **to_process) {
    free(*to_process);
    *to_process = NULL;
}



/********************/
/*** Number tools ***/
/********************/

/**
 * Counts the number of digits in the given integer (including the negative sign, if applicable).
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
