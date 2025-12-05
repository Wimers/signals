#ifndef MAIN_H
#define MAIN_H

// Included Libraries
#include "fileParsing.h"
#include <stdint.h>
#include <stdio.h>
#include <getopt.h>

// Exit codes
#define EXIT_OK 0
#define EXIT_EMPTY_ARG 19
#define EXIT_INVALID_ARG 20
#define EXIT_FILE_CANNOT_BE_READ 9
#define EXIT_OUTPUT_FILE_ERROR 11
#define EXIT_NO_COMMAND 6
#define EXIT_MISSING_INPUT_FILE 88
#define EXIT_INVALID_FILE_TYPE 89
#define EXIT_INVALID_PARAMETER 90
#define EXIT_SAME_FILE 91

#define MIN_CMD_ARGS 2

// Program constant strings
extern const char* const usageMessage;
extern const char* const commandHelpMessage;
extern const char* const fileTypeMessage;
extern const char* const fileOpeningErrorMessage;
extern const char* const invalidArgsMessage;
extern const char* const lineSeparator;
extern const char* const suXFormat;
extern const char* const sdXFormat;
extern const char* const sssFormat;
extern const char* const ssdFormat;
extern const char* const sudFormat;
extern const char* const fileType;
extern const char* const optstring;
extern const char* const gotStrMessage;
extern const char* const gotIntMessage;
extern const char* const emptyArgsMessage;
extern const char* const noArgsProvidedMessage;
extern const char* const userHelpPrompt;
extern const char* const unexpectedArgMessage;
extern const char* const glitchOffsetValMessage;
extern const char* const glitchUsageMessage;

// Assorted constant chars
extern const char* const readMode;
extern const char* const writeMode;
#define EOS '\0'

typedef enum {
    INVALID = -1,
    HELP = 'h',
    DUMP_HEADER = 'd',
    PRINT_IMAGE = 'p',
    INPUT_FILE = 'i',
    OUTPUT_FILE = 'o',
    FILTERS = 'f',
    GRAY_SCALE = 'g',
    INVERT = 'v',
    FLIP = 'u',
    BRIGHTNESS_CAP = 'b',
    COMBINE = 'c',
    GLITCH = 'l',
    AVE = 'a',
    CONTRAST = 't',
    DIM = 'm',
} Flag;

typedef struct {
    uint8_t input;
    char* inputFilePath;
    uint8_t output;
    char* outputFilePath;
    char* filters;
    uint8_t help;
    uint8_t header;
    uint8_t print;
    uint8_t invert;
    uint8_t filter;
    uint8_t grayscale;
    uint8_t flip;
    uint8_t maxBrightness;
    uint8_t combine;
    char* combineFilePath;
    int32_t glitch;
    uint8_t average;
    uint8_t contrast;
    uint8_t dim;
} UserInput;

/* check_for_empty_args()
 * ----------------------
 * argc:
 * argv:
 *
 * Returns:
 */
int check_for_empty_args(const int argc, char** argv);

/* early_argument_checks()
 * -----------------------
 *  argc:
 *  argv:
 *
 *  Returns:
 */
int early_argument_checks(const int argc, char** argv);

/* parse_user_commands()
 * ---------------------
 * argc:
 * argv:
 *
 * Returns:
 */
int parse_user_commands(const int argc, char** argv, UserInput* userInput);

/* check_each_char_is_digit()
 * --------------------------
 * arg:
 *
 * Returns:
 */
int check_each_char_is_digit(const char* arg);

/* check_int_within_bounds()
 * -------------------------
 * num:
 * min:
 * max:
 *
 * Returns:
 */
int check_int_within_bounds(const int num, const int min, const int max);

/* verify_int_arg_with_bounds()
 * ----------------------------
 * arg:
 * min:
 * max:
 *
 * Returns:
 */
int verify_int_arg_with_bounds(const char* arg, const int min, const int max);

/* verify_glitch_arg()
 * -------------------
 * userInput:
 * arg:
 *
 * Returns:
 */
int verify_glitch_arg(UserInput* userInput, const char* arg);

/* glitch_offset_invalid_message()
 * -------------------------------
 * arg:
 *
 * Returns:
 */
int glitch_offset_invalid_message(const char* arg);

/* handle_commands()
 * -----------------
 * userInput:
 *
 * Returns:
 */
int handle_commands(UserInput* userInput);

/* handle_combine()
 * ----------------
 * userInput:
 * bmpImage:
 *
 * Returns:
 */
int handle_combine(const UserInput* userInput, BMP* bmpImage);

/* ends_with()
 * -----------
 * target:
 * arg:
 *
 * Returns:
 */
int ends_with(const char* const target, const char* arg);

/* check_valid_file_type()
 * -----------------------
 * filePath:
 *
 * Returns:
 */
int check_valid_file_type(const char* filePath);

#endif
