// Included Libraries
#include "fileParsing.h"
#include "filters.h"
#include "main.h"
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>

// Error messages
const char* const fileTypeMessage = "Input file must be \".bmp\"\n";
const char* const invalidArgsMessage = "Invalid arguments supplied.\n";
const char* const emptyArgsMessage = "Arguments must not be empty.\n";
const char* const noArgsProvidedMessage = "An argument must be supplied.\n";
const char* const userHelpPrompt
        = "Enter: \"./bmp --help\" for available commands\n";
const char* const unexpectedArgMessage = "Got \"%s\", expected \"%s\"\n";
const char* const glitchOffsetValMessage
        = "\nOffset must be a positive integer, within input image "
          "bounds.\n";
const char* const gotStrMessage = "    Got \"%s\".\n";
const char* const glitchUsageMessage
        = "Glitch Effect\nUsage:  -l, --glitch <offset>  "
          "    - Apply horizontal glitch effect\n";
const char* const nonUniquePathsMessage
        = "Input and combine file paths must be unique!\n";

// Program constant strings
const char* const usageMessage = "Usage: ./bmp <option> [--input <file>] ...\n";
const char* const helpMessage // Need to update
        = "Usage: ./bmp <option> [--input <file>] ...\n"
          "\n"
          "Help:\n"
          "  -h, --help                  - Show this help message\n"
          "\n"
          "File I/O Options:\n"
          "  -i, --input <file>          - Input BMP file to process\n"
          "  -o, --output <file>         - Output file path to write result\n"
          "  -d, --dump                  - Dump BMP header information to "
          "terminal\n"
          "  -p, --print                 - Render image to terminal (ANSI)\n"
          "\n"
          "Image Filters:\n"
          "  -g, --grayscale             - Convert image to grayscale\n"
          "  -v, --invert                - Invert image colours\n"
          "  -f, --filter                - Apply red channel isolation\n"
          "  -u, --flip                  - Flip image vertically\n"
          "  -b, --brightness-cap <val>  - Cap pixel brightness (0-255)\n"
          "\n"
          "Advanced Effects:\n"
          "  -l, --glitch <offset>       - Apply horizontal glitch effect\n"
          "  -c, --combine <file>        - Overlay another BMP image onto "
          "input\n";

const char* const lineSeparator
        = "--------------------------------------------------\n";
const char* const fileType = ".bmp";
const char* const optstring = "i:o:b:c:l:t:dphfgvua"; // Defined program flags

// Assorted constant chars
const char* const readMode = "rb";
const char* const writeMode = "wb";

static struct option const longOptions[] = {
        {"input", required_argument, NULL, INPUT_FILE},
        {"output", required_argument, NULL, OUTPUT_FILE},
        {"dump", no_argument, NULL, DUMP_HEADER},
        {"print", no_argument, NULL, PRINT_IMAGE},
        {"help", no_argument, NULL, HELP},
        {"filter", no_argument, NULL, FILTERS},
        {"grayscale", no_argument, NULL, GRAY_SCALE},
        {"invert", no_argument, NULL, INVERT},
        {"flip", no_argument, NULL, FLIP},
        {"brightness-cap", required_argument, NULL, BRIGHTNESS_CAP},
        {"combine", required_argument, NULL, COMBINE},
        {"glitch", required_argument, NULL, GLITCH},
        {"average", no_argument, NULL, AVE},
        {"contrast", required_argument, NULL, CONTRAST},
        {"dim", required_argument, NULL, DIM},
        {NULL, 0, NULL, 0},
};

int main(const int argc, char* argv[])
{
    if (early_argument_checks(argc, argv) != EXIT_SUCCESS) {
        exit(EXIT_INVALID_ARG);
    }

    UserInput userInput;
    (void)memset(&userInput, 0, sizeof(userInput));

    int status;
    if ((status = parse_user_commands(argc, argv, &userInput))
            != EXIT_SUCCESS) {
        exit(status);
    }

    exit(handle_commands(&userInput));
}

int check_for_empty_args(const int argc, char** argv)
{
    for (int i = 0; i < argc; i++) {
        if (argv[i][0] == EOS) {
            fputs(emptyArgsMessage, stderr);
            fputs(usageMessage, stderr);
            return -1;
        }
    }

    return EXIT_SUCCESS;
}

int early_argument_checks(const int argc, char** argv)
{
    if (!(argc >= MIN_CMD_ARGS)) {
        fputs(noArgsProvidedMessage, stderr);
        fputs(usageMessage, stderr);
        fputs(userHelpPrompt, stderr);
        return -1;
    }

    if (check_for_empty_args(argc, argv) == -1) {
        return -1;
    }

    return EXIT_SUCCESS;
}

int parse_user_commands(const int argc, char** argv, UserInput* userInput)
{
    Flag opt;

    // loop over all of the options
    while ((opt = getopt_long(argc, argv, optstring, longOptions, NULL))
            != -1) {
        switch (opt) {

        case HELP:
            userInput->help = 1;
            break;

        case DUMP_HEADER:
            userInput->header = 1;
            break;

        case PRINT_IMAGE:
            userInput->print = 1;
            break;

        case INPUT_FILE:
            userInput->input = 1;
            userInput->inputFilePath = optarg;
            break;

        case OUTPUT_FILE:
            userInput->output = 1;
            userInput->outputFilePath = optarg;
            break;

        case FILTERS:
            userInput->filter = 1;
            userInput->filters = optarg;
            break;

        case GRAY_SCALE:
            userInput->grayscale = 1;
            break;

        case INVERT:
            userInput->invert = 1;
            break;

        case FLIP:
            userInput->flip = 1;
            break;

        case BRIGHTNESS_CAP: {
            long cap;
            if ((cap = verify_long_arg_with_bounds(optarg, 0, UINT8_MAX))
                    == -1) {
                return EXIT_INVALID_PARAMETER;
            }
            userInput->maxBrightness = (uint8_t)cap;
            break;
        }

        case COMBINE:
            userInput->combine = 1;
            userInput->combineFilePath = optarg;
            break;

        case GLITCH:
            if (verify_glitch_arg(userInput, optarg) == -1) {
                return EXIT_INVALID_PARAMETER;
            }
            break;

        case AVE:
            userInput->average = 1;
            break;

        case CONTRAST: {
            long contrast;
            if ((contrast = verify_long_arg_with_bounds(optarg, 0, UINT8_MAX))
                    == -1) {
                return EXIT_INVALID_PARAMETER;
            }
            userInput->contrast = (uint8_t)contrast;
            break;
        }

        case DIM: {
            long dim;
            if ((dim = verify_long_arg_with_bounds(optarg, 0, UINT8_MAX))
                    == -1) {
                return EXIT_INVALID_PARAMETER;
            }
            userInput->dim = (uint8_t)dim;
            break;
        }

        default:
            return EXIT_NO_COMMAND;
        }
    }

    return EXIT_SUCCESS;
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

long verify_long_arg_with_bounds(
        const char* arg, const long min, const long max)
{
    char* endptr;
    errno = 0;

    const long val = strtol(arg, &endptr, BASE_10);

    if ((arg == endptr) || (*endptr != EOS)) {
        return -1;
    }

    if (errno == ERANGE) {
        return -1;
    }

    // Check number within specified bounds
    if (check_long_within_bounds(val, min, max) == -1) { // Returns -1 on error
        return -1;
    }

    return val;
}

int check_long_within_bounds(const long num, const long min, const long max)
{
    // Check number within specified bounds
    if ((num < min) || (num > max)) {
        return -1;
    }

    return EXIT_SUCCESS;
}

int verify_glitch_arg(UserInput* userInput, const char* arg)
{
    long glitchOffset;

    if ((glitchOffset = verify_long_arg_with_bounds(arg, 1, INT32_MAX)) == -1) {

        // Prints error messages
        glitch_offset_invalid_message(arg);
        return -1;
    }

    userInput->glitch = (int32_t)glitchOffset;
    return EXIT_SUCCESS;
}

void glitch_offset_invalid_message(const char* arg)
{
    fputs(glitchUsageMessage, stderr);
    fputs(glitchOffsetValMessage, stderr);
    fprintf(stderr, gotStrMessage, arg);
}

int handle_commands(UserInput* userInput)
{
    if (userInput->help) {
        fputs(helpMessage, stdout);
    }

    if (!(userInput->input)) {
        return EXIT_MISSING_INPUT_FILE;
    }
    if (check_valid_file_type(userInput->inputFilePath) == -1) {
        return EXIT_INVALID_FILE_TYPE;
    }

    BMP bmpImage;
    initialise_bmp(&bmpImage);
    int status;

    if ((status = open_bmp(&bmpImage, userInput->inputFilePath))
            != EXIT_SUCCESS) {
        free_image_resources(&bmpImage);
        return status;
    }

    if (userInput->header) {
        dump_headers(&bmpImage);
    }

    if ((status = handle_bmp_loading(&bmpImage)) != EXIT_SUCCESS) {
        free_image_resources(&bmpImage);
        return status;
    }

    if (!userInput->flip) {
        flip_image(bmpImage.image);
    }

    if (userInput->filter) {
        filter_green(bmpImage.image);
    }

    if (userInput->maxBrightness) {
        brightness_cap_filter(bmpImage.image, userInput->maxBrightness);
    }

    if (userInput->grayscale) {
        gray_filter(bmpImage.image);
    }

    if (userInput->glitch) {
        if (glitch_effect(bmpImage.image, userInput->glitch) == -1) {
            free_image_resources(&bmpImage);
            return EXIT_OUT_OF_BOUNDS; // FIX
        }
    }

    if (userInput->contrast) {
        contrast_effect(bmpImage.image, userInput->contrast, 100, 160);
    }

    if (userInput->dim) {
        dim_effect(bmpImage.image, userInput->dim);
    }

    if (userInput->combine) {
        if ((status = handle_combine(userInput, &bmpImage)) != EXIT_SUCCESS) {
            free_image_resources(&bmpImage);
            return status;
        }
    }

    if (userInput->invert) {
        invert_colours(bmpImage.image);
    }

    if (userInput->average) {
        average_pixels(bmpImage.image);
    }

    if (userInput->output) {
        if (write_bmp_with_header_provided(&bmpImage, userInput->outputFilePath)
                == -1) {
            return EXIT_OUTPUT_FILE_ERROR;
        }
    }

    if (userInput->print) {
        print_image_to_terminal(bmpImage.image);
    }

    free_image_resources(&bmpImage);

    return EXIT_SUCCESS;
}

int handle_combine(const UserInput* userInput, BMP* bmpImage)
{
    if (check_valid_file_type(userInput->combineFilePath) == -1) {
        return EXIT_INVALID_FILE_TYPE;
    }

    // Exit if input and combine file paths match
    if (!strcmp(userInput->inputFilePath, userInput->combineFilePath)) {
        fputs(nonUniquePathsMessage, stderr);
        return EXIT_SAME_FILE;
    }

    BMP combinedImage;
    initialise_bmp(&combinedImage);

    int status;
    if ((status = open_bmp(&combinedImage, userInput->combineFilePath))
            != EXIT_SUCCESS) {
        free_image_resources(&combinedImage);
        return status;
    }

    if ((status = handle_bmp_loading(bmpImage)) != EXIT_SUCCESS) {
        free_image_resources(&combinedImage);
        return status;
    }

    flip_image(combinedImage.image);

    if ((status = combine_images(bmpImage->image, combinedImage.image))
            != EXIT_SUCCESS) {
        free_image_resources(&combinedImage);
        return status;
    }

    free_image_resources(&combinedImage);
    return EXIT_SUCCESS;
}

int ends_with(const char* const target, const char* arg)
{
    const size_t lenArg = strlen(arg);
    const size_t lenTarget = strlen(target);

    if (lenArg < lenTarget) {
        return -1;
    }

    return !(strcmp(target, &(arg[lenArg - lenTarget])));
}

int check_valid_file_type(const char* filePath)
{
    if (!ends_with(fileType, filePath)) {
        fputs(fileTypeMessage, stderr);
        fprintf(stderr, unexpectedArgMessage, filePath, fileType);
        return -1;
    }

    return EXIT_SUCCESS;
}
