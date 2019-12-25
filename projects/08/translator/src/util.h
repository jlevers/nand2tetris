#ifndef _VM_UTIL_H
#define _VM_UTIL_H

extern const char DIR_SEP;    // The character used to separate directories in a file path

// Stores a format string and the length of all format specifiers in it
typedef struct fmt_str {
    // The format string
    char *str;
    // The character length of all format specifiers in str
    int fmt_len;
} fmt_str;

// Represents the parts of a a filesystem path.
// Basename and dirname here refer to the *nix meaning: http://man7.org/linux/man-pages/man3/basename.3.html
// I could just do this by including libgen.h, but I thought it'd be more fun to write it myself.
typedef struct path_parts {
    char *dirname;   // The path up to, but not including, the basename of a path
    char *basename;  // The basename of a path
} path_parts;

int is_directory(char*);
void path_parts_split(path_parts*, char*);
int path_parts_cmp(path_parts*, path_parts*);
void path_parts_delete(path_parts**);
char *remove_fext(char*);
void toupper_str(char*, char*);
int vm_strcmp(char*, char*);
fmt_str *fmt_str_new(const char*, int);
int fmt_str_len(const fmt_str*);
void fmt_str_delete(fmt_str**);
void reinit_char(char**);
int num_digits(int);

#endif /* _VM_UTIL_H */
