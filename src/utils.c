#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

int is_str_in_const_str_array(const void* restrict arg,
        const char* const strArray[], const size_t nRead)
{
    // Initialise
    const int breakPoint = 1000;
    int i = 0;

    // For each string in string array
    while (strArray[i] != NULL) {

        // Check if arg is equal to this string
        if ((strlen(strArray[i]) >= nRead)
                && (!memcmp(arg, strArray[i], nRead))) {

            return EXIT_SUCCESS; // Arg found
        }

        if (++i > breakPoint) {
            return -1;
        }
    }

    // Arg is not an element of the string array
    return -1;
}

int ends_with(const char* const suffix, const char* arg)
{
    // Initialise
    const size_t lenArg = strlen(arg);
    const size_t lenSuffix = strlen(suffix);

    // Check argument isn't smaller than the suffix
    if (lenArg < lenSuffix) {
        return -1;
    }

    // Returns 1 if arg does end with the target, else returns 0.
    return !(strcmp(suffix, &(arg[lenArg - lenSuffix])));
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

    if ((arg == endptr) || (*endptr != '\0')) {
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

int* separate_to_int_array(const char* str, const char delim, const size_t exp)
{
    if ((delim == '-') || (delim == '+')) {
        return NULL;
    }

    if ((str == NULL) || (exp == 0)) {
        return NULL;
    }

    if (str[0] == delim) {
        return NULL;
    }

    size_t len = strlen(str);
    if (len < 1) {
        return NULL;
    }

    int* values = calloc(exp, sizeof(int));
    if (values == NULL) {
        return NULL;
    }

    const char* cur = str;
    size_t vCount = 0;
    size_t i = 0;

    while (str[i] != '\0') {
        if (str[i] == delim) {
            if (vCount >= exp) {
                free(values);
                return NULL;
            }

            values[vCount] = atoi(cur);
            vCount++;

            cur = &(str[i + 1]);
        }
        i++;
    }

    if (*cur != '\0') {
        if (vCount >= exp) {
            free(values);
            return NULL;
        }

        values[vCount] = atoi(cur);
        vCount++;
    }

    if (vCount != exp) {
        free(values);
        return NULL;
    }

    return values;
}
