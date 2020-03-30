/* file: minunit.h */
/* Copyright (C) 2002 John Brewer */
/* You may use this code for any purpose, with the understanding that it comes with NO WARRANTY. */

#define mu_assert(message, test) do {\
    if (!(test)) {\
        return message;\
    } else {\
        tests_run++;\
    }\
} while (0)
#define mu_run_test(test) do { char *message = test(); if (message) return message; } while (0)
extern int tests_run;
