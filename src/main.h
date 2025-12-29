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

#define gotIntMessage "    Got \"%d\".\n"

constexpr char glitchOffsetValMessage[]
        = "\nOffset must be a positive integer, within input image "
          "bounds.\n";
constexpr char glitchUsageMessage[]
        = "Glitch Effect\nUsage:  -l, --glitch <offset>  "
          "    - Apply horizontal glitch effect\n";

// Assorted constant chars
extern const char* const readMode;
extern const char* const writeMode;

typedef struct {
    uint8_t input;
    char* inputFilePath;
    uint8_t output;
    char* outputFilePath;
    uint8_t help;
    bool header;
    bool print;
    bool invert;
    uint8_t filters;
    uint8_t grayscale;
    bool flip;
    uint8_t cutoff;
    bool combine;
    char* combineFilePath;
    size_t glitch;
    uint8_t average;
    uint8_t contrast;
    uint8_t dim;
    bool swap; // FIX
    long rotations;
    bool reverse;
    bool melt;
    int32_t meltOffset;
    double scale;
    bool merge;
    char* mergeFilePath;
    uint32_t blur;
    char* encodeFilePath;
} UserInput;

typedef void (*ColourFilter)(Image* image);
typedef void (*Function)(void);

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

uint8_t handle_colour_filter_arg_parsing(const char* arg);
Function handle_colour_filters(const uint8_t filters);
void handle_image_rotation(BMP* bmpImage, const long nRotations);
int handle_merge(const UserInput* userInput, BMP* bmpImage);

#endif
