#ifndef MAIN_H
#define MAIN_H

// Included Libraries
#include "fileParsing.h"
#include <stdint.h>
#include <stdio.h>
#include <getopt.h>

// Exit codes
#define EXIT_OK 0
#define EXIT_INVALID_ARG 20
#define EXIT_FILE_CANNOT_BE_READ 9
#define EXIT_NO_COMMAND 6

// cmd line argument indexes
#define FILE_PATH_IX 2

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

// Assorted constant chars
extern const char* const readMode;
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
} UserInput;

/* print_bmp_header()
 * ------------------
 * Prints the values of each parameter from the BMP Info header parsed.
 *
 * bmp: Struct containing all parsed BMP Header metadata.
 */
void print_bmp_header(const BmpHeader* bmp);

/* print_bmp_info_header()
 * -----------------------
 * Prints the values of each parameter from the BMP Info header parsed.
 *
 * bmp: Pointer to struct containing all parsed BMP Info Header metadata.
 */
void print_bmp_info_header(const BmpInfoHeader* bmp);

void check_file_opened(FILE* file, const char* const filePath);
void early_argument_checks(const int argc, char** argv);
void check_for_empty_args(const int argc, char** argv);
void check_argument_validity(const int argc, char** argv);
Flag command_mapping(const char* command);
int ends_with(const char* const target, const char* arg);
void early_argument_checks(const int argc, char** argv);
void handle_commands(UserInput* userInput);
void parse_user_commands(const int argc, char** argv, UserInput* userInput);

#endif
