#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stdlib.h>

constexpr char eos = '\0';
constexpr int base10 = 10;

int is_str_in_const_str_array(const void* restrict arg,
        const char* const strArray[], const size_t nread);

static inline size_t fast_u8_to_buf(char* buf, uint8_t val)
{
    if (val >= 100) {
        uint8_t d1 = val / 100;
        buf[0] = (char)('0' + d1);

        val = val - (uint8_t)(d1 * 100);
        uint8_t d2 = val / 10;
        buf[1] = (char)('0' + d2);

        val = val - (uint8_t)(d2 * 10);
        buf[2] = (char)('0' + val);
        return 3;

    } else if (val >= 10) {
        uint8_t d2 = val / 10;
        buf[0] = (char)('0' + d2);

        val = val - (uint8_t)(d2 * 10);
        buf[1] = (char)('0' + val);
        return 2;

    } else {
        buf[0] = (char)('0' + val);
        return 1;
    }
}

/* ends_with()
 * -----------
 * Checks if an input string ends in a target string.
 *
 * target: String expected at end of arg string.
 * arg: Input string.
 *
 * Returns: 1 if the string does end with target, -1 if the input target string
 *          is larger than the arg string, and 0 otherwise.
 */
int ends_with(const char* const target, const char* arg);

/* check_each_char_is_digit()
 * --------------------------
 * Checks each character in a string is a digit (0 <-> 9).
 *
 * argv: String argument input.
 *
 * Returns: EXIT_SUCCESS if each character is a digit, else returns -1 on
 *          error.
 */
int check_each_char_is_digit(const char* arg);

/* check_long_within_bounds()
 * --------------------------
 * Checks if a number is between a minimum and maximum value. The boundry
 * includes the boarder values.
 *
 * num: Number to check.
 * min: Minimum bound on number.
 * max: Maximum bound on number.
 *
 * Returns: EXIT_SUCCESS if within bounds.
 *
 * Errors: Returns -1.
 */
int check_long_within_bounds(const long num, const long min, const long max);

/* verify_long_arg_with_bounds()
 * ----------------------------
 * Checks whether an input string represents a base 10 number, within specified
 * upper and lower bounds.
 *
 * arg: String argument input.
 * min: Minimum bound on number.
 * max: Maximum bound on number.
 *
 * Returns: EXIT_SUCCESS if within bounds.
 *
 * Errors: Returns -1.
 */
long verify_long_arg_with_bounds(
        const char* arg, const long min, const long max);

#endif
