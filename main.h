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
#define BASE_10 10

#define gotIntMessage "    Got \"%d\".\n"

// Program constant strings
extern const char* const usageMessage;
extern const char* const commandHelpMessage;
extern const char* const fileTypeMessage;
extern const char* const fileOpeningErrorMessage;
extern const char* const invalidArgsMessage;
extern const char* const lineSeparator;
extern const char* const nonUniquePathsMessage;
extern const char* const fileType;
extern const char* const optstring;
extern const char* const gotStrMessage;
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
    SWAP = 's',
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
    size_t glitch;
    uint8_t average;
    uint8_t contrast;
    uint8_t dim;
    uint8_t swap; // FIX
} UserInput;

/* check_for_empty_args()
 * ----------------------
 * Searches an array of strings for empty string arguments.
 * Prints an error message to stderr if an empty string is found.
 *
 * argc: Number of command line arguments.
 * argv: Array of of command line argument strings.
 *
 * Returns: EXIT_SUCCESS if no empty args found, -1 otherwise.
 */
int check_for_empty_args(const int argc, char** argv);

/* early_argument_checks()
 * -----------------------
 * Checks list of command line arguments contains at least 2 arguments,
 * and none are empty strings.
 *
 * argc: Number of command line arguments.
 * argv: Array of of command line argument strings.
 *
 * Returns: EXIT_SUCCESS if checks pass, -1 otherwise.
 */
int early_argument_checks(const int argc, char** argv);

/* parse_user_commands()
 * ---------------------
 * Parses an array of command line arguments to determine which options
 * the program is to run with. Verifies the validity of additional arguments
 * associated with specific flags/
 *
 * argc: Number of command line arguments.
 * argv: Array of of command line argument strings.
 *
 * Returns: EXIT_SUCCESS if arguments parses successfully.
 *
 * Errors: Returns EXIT_NO_COMMAND or EXIT_INVALID_PARAMETER on error, these
 *         occur when an unknown flag is provided or when an invalid flag
 *         specific argument is provided respectively.
 */
int parse_user_commands(const int argc, char** argv, UserInput* userInput);

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

/* verify_int_arg_with_bounds()
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

/* glitch_offset_invalid_message()
 * -------------------------------
 * Prints messages to stderr indicating the command line argument provided with
 * the glitch flag was invalid.
 *
 * arg: Argument user supplied.
 */
void glitch_offset_invalid_message(const char* arg);

/* handle_commands()
 * -----------------
 * Manages the full lifecycle of the program after command line parsing.
 * Validates input, applies requested transformations to the image data,
 * generates output, and frees resources.
 *
 * userInput: Pointer to struct containing parsed user options.
 *
 * Returns: EXIT_SUCCESS on successful handling of all commands, or returns a
 *          specific error code.
 */
int handle_commands(UserInput* userInput);

/* handle_combine()
 * ----------------
 * Manages the logic for the combine/overlay effect. Opens the secondary
 * image file specified by the user, validates it, and merges it with the
 * primary source image.
 *
 * userInput: Users input arguments containing file path for file to combine.
 * bmpImage: Pointer to the primary image to overlay onto.
 *
 * Returns: EXIT_SUCCESS on success, or a specific error code.
 */
int handle_combine(const UserInput* userInput, BMP* bmpImage);

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

/* check_valid_file_type()
 * -----------------------
 * Checks if a file is of the type specified.
 *
 * type: String representing the valid file type.
 * filePath: File path of input file.
 *
 * Returns: EXIT_SUCCESS if file type is valid, else -1.
 */
int check_valid_file_type(const char* const type, const char* filePath);

#endif
