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
const char* const usageMessage = "Options: ./bmp [-h] [-d] [-i] <file>\n";
const char* const fileTypeMessage = "Input file must be \".bmp\"\n";
const char* const fileOpeningErrorMessage
        = "The provided file \"%s\" cannot be opened for reading\n";
const char* const invalidArgsMessage = "Invalid arguments supplied\n";
const char* const lineSeparator
        = "--------------------------------------------------\n";
const char* const suXFormat = "%-25s %-15u %X\n";
const char* const sdXFormat = "%-25s %-15d %X\n";
const char* const sssFormat = "%-25s %-15s %s\n";
const char* const ssdFormat = "%-25s %-15s %d\n";
const char* const sudFormat = "%-25s %-15u %d\n";
const char* const fileType = ".bmp";

// Assorted constant chars
const char* const readMode = "r";
#define EOS '\0'

const char* const optstring = "i:o:dphfgvu"; // Defined program flags

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
} Flag;

static struct option long_options[] = {
        {"input", required_argument, NULL, INPUT_FILE},
        {"output", required_argument, NULL, OUTPUT_FILE},
        {"dump", no_argument, NULL, DUMP_HEADER},
        {"print", no_argument, NULL, PRINT_IMAGE},
        {"help", no_argument, NULL, HELP},
        {"filter", required_argument, NULL, FILTERS},
        {"grayscale", no_argument, NULL, GRAY_SCALE},
        {"invert", no_argument, NULL, INVERT},
        {"flip", no_argument, NULL, FLIP},
        {"brightness-cap", required_argument, NULL, BRIGHTNESS_CAP},
        {NULL, 0, NULL, 0},
};

typedef struct {
    char* inputFilePath;
    char* outputFilePath;
    char* filters;
    uint8_t maxBrightness;
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
void read_headers(BmpHeader* restrict bmp, BmpInfoHeader* restrict infoHeader,
        FILE* file);
void check_for_empty_args(const int argc, char** argv);
void check_argument_validity(const int argc, char** argv);
Flag command_mapping(const char* command);
int ends_with(const char* const target, const char* arg);
void early_argument_checks(const int argc, char** argv);

#endif
