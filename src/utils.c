#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

int is_str_in_const_str_array(const void* restrict arg,
        const char* const strArray[], const size_t nread)
{
    // Initialise
    const int breakPoint = 1000;
    int i = 0;

    // For each string in string array
    while (strArray[i] != NULL) {

        // Check if arg is equal to this string
        if ((strlen(strArray[i]) >= nread)
                && (!memcmp(arg, strArray[i], nread))) {

            return EXIT_SUCCESS; // Arg found
        }

        if (++i > breakPoint) {
            return -1;
        }
    }

    // Arg is not an element of the string array
    return -1;
}

int ends_with(const char* const target, const char* arg)
{
    // Initialise
    const size_t lenArg = strlen(arg);
    const size_t lenTarget = strlen(target);

    // Check argument isn't smaller than the target
    if (lenArg < lenTarget) {
        return -1;
    }

    // Returns 1 if arg does end with the target, else returns 0.
    return !(strcmp(target, &(arg[lenArg - lenTarget])));
}

int check_each_char_is_digit(const char* arg)
{
    const size_t len = strlen(arg);

    // For each character in input string
    for (size_t i = 0; i < len; i++) {

        // If char is not a digit
        if (!isdigit((unsigned char)arg[i])) {
            return -1;
        }
    }

    return EXIT_SUCCESS;
}

int check_long_within_bounds(const long num, const long min, const long max)
{
    // Check number within specified bounds
    if ((num < min) || (num > max)) {
        return -1;
    }

    return EXIT_SUCCESS;
}

bool vlongB_check(long* output, const char* arg, const long min, const long max)
{
    char* endptr;
    errno = 0;

    // Convert arg string to a long.
    const long val = strtol(arg, &endptr, base10);

    if ((arg == endptr) || (*endptr != eos)) {
        return false;
    }

    if (errno == ERANGE) {
        return false;
    }

    // Check number within specified bounds
    if (check_long_within_bounds(val, min, max) == -1) { // Returns -1 on error
        return false;
    }

    *output = val;
    return true;
}
