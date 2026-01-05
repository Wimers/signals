#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stdlib.h>

constexpr int base10 = 10;

/* vlongB()
 * --------
 * Macro to safely parse a string into a numeric type with bounds checking.
 *
 * output: Pointer to the variable to store the result.
 * arg: String to parse.
 * min: Minimum allowed value.
 * max: Maximum allowed value.
 * type: The type of the output variable (e.g., int, long).
 *
 * Returns: true if successful, false otherwise.
 */
#define vlongB(output, arg, min, max, type)                                    \
    __extension__({                                                            \
        long _temp;                                                            \
        bool _success = vlongB_check(&(_temp), arg, min, max);                 \
        if (_success) {                                                        \
            *(type*)(output) = (type)_temp;                                    \
        }                                                                      \
        _success;                                                              \
    })

/* fast_u8_to_buf()
 * ----------------
 * Converts a uint8_t value to its decimal string representation and stores it
 * in the provided buffer.
 *
 * buf: Buffer to store the digits.
 * val: The value to convert.
 *
 * Returns: The number of digits written to the buffer (1-3).
 *
 * Note
 * ----
 * It is assumed the buffer to store the digits is at least 4 bytes, and has
 * been initialised to zero prior to calling, so that the output is NULL
 * terminated.
 */
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

/* is_str_in_const_str_array()
 * ---------------------------
 * Checks if a string is present within a NULL terminated array of strings.
 * Comparison is limited to nRead bytes.
 *
 * arg: String to target.
 * strArray: NULL terminated array of strings.
 * nRead: Maximum number of bytes to compare.
 *
 * Returns: EXIT_SUCCESS if found, -1 otherwise.
 */
[[nodiscard]] int is_str_in_const_str_array(const void* restrict arg,
        const char* const strArray[], const size_t nRead);

/* ends_with()
 * -----------
 * Checks if an input string ends in a supplied suffix.
 *
 * suffix: String suffix.
 * arg: Input string.
 *
 * Returns: 1 if the string does end with the suffix, -1 if the suffix string
 *          is larger than the arg string, and 0 otherwise.
 */
[[nodiscard]] int ends_with(const char* const suffix, const char* arg);

/* check_each_char_is_digit()
 * --------------------------
 * Checks if each character in a string is a digit (0 <-> 9).
 *
 * arg: String argument input.
 *
 * Returns: EXIT_SUCCESS if valid, else -1.
 */
[[nodiscard]] int check_each_char_is_digit(const char* arg);

/* check_long_within_bounds()
 * --------------------------
 * Checks if a number is within a specified range [min, max].
 *
 * num: Number to check.
 * min: Minimum allowed value.
 * max: Maximum allowed value.
 *
 * Returns: EXIT_SUCCESS if within bounds, else -1.
 */
[[nodiscard]] int check_long_within_bounds(
        const long num, const long min, const long max);

/* vlongB_check()
 * --------------
 * Helper function for vlongB macro. Parses a string to a long and verifies
 * it falls within the specified bounds.
 *
 * output: Pointer to store the parsed long value.
 * arg: String to parse.
 * min: Minimum allowed value.
 * max: Maximum allowed value.
 *
 * Returns: true if parsing and bounds check succeed, false otherwise.
 */
[[nodiscard]] bool vlongB_check(
        long* output, const char* arg, const long min, const long max);

#endif
