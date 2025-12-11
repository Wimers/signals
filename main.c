// Included Libraries
#include "fileParsing.h"
#include "filters.h"
#include "imageEditing.h"
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
const char* const invalidFilterColourMessage
        = "Filter colour/s invalid \"%s\", must be RGB characters.\n";

// Program constant strings
const char* const usageMessage = "Usage: ./bmp <option> [--input <file>] ...\n";
const char* const helpMessage // Need to update FIX
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
const char* const optstring
        = "i:o:b:c:l:t:r:dphfgvuas"; // Defined program flags

// Assorted constant chars
const char* const readMode = "rb";
const char* const writeMode = "wb";

static struct option const longOptions[] = {
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
        {"combine", required_argument, NULL, COMBINE},
        {"glitch", required_argument, NULL, GLITCH},
        {"average", no_argument, NULL, AVE},
        {"contrast", required_argument, NULL, CONTRAST},
        {"dim", required_argument, NULL, DIM},
        {"swap", no_argument, NULL, SWAP},
        {"rotate", required_argument, NULL, ROTATE},
        {NULL, 0, NULL, 0},
};

int main(const int argc, char* argv[])
{
    if (early_argument_checks(argc, argv) != EXIT_SUCCESS) {
        exit(EXIT_INVALID_ARG);
    }

    // Initialise
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
    // Check if user did not supply any arguments
    if (!(argc >= MIN_CMD_ARGS)) {

        // Print error messages
        fputs(noArgsProvidedMessage, stderr);
        fputs(usageMessage, stderr);
        fputs(userHelpPrompt, stderr);
        return -1;
    }

    if (check_for_empty_args(argc, argv) == -1) {
        return -1;
    }

    // Checks passed
    return EXIT_SUCCESS;
}

int parse_user_commands(const int argc, char** argv, UserInput* userInput)
{
    Flag opt;

    // Loop over all of the options
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
            userInput->filters = handle_colour_filter_arg_parsing(optarg);
            if (userInput->filters == 0) {
                return EXIT_INVALID_PARAMETER;
            }
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

        case BRIGHTNESS_CAP: { // If brightness cap flag used, verify the
                               // required argument
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

        case GLITCH: { // If glitch flag used, verify the glitch offset
            long glitchOff;
            if ((glitchOff = verify_long_arg_with_bounds(optarg, 1, INT32_MAX))
                    == -1) {
                glitch_offset_invalid_message(optarg); // Prints error messages
                return EXIT_INVALID_PARAMETER;
            }
            userInput->glitch = (size_t)glitchOff;
            break;
        }

        case AVE:
            userInput->average = 1;
            break;

        case CONTRAST: { // If contrast flag set, verify the required argument
            long contrast;
            if ((contrast = verify_long_arg_with_bounds(optarg, 0, UINT8_MAX))
                    == -1) {
                return EXIT_INVALID_PARAMETER;
            }
            userInput->contrast = (uint8_t)contrast;
            break;
        }

        case DIM: { // If dim flag set, verify the required argument
            long dim;
            if ((dim = verify_long_arg_with_bounds(optarg, 0, UINT8_MAX))
                    == -1) {
                return EXIT_INVALID_PARAMETER;
            }
            userInput->dim = (uint8_t)dim;
            break;
        }

        case SWAP:
            userInput->swap = 1;
            break;

        case ROTATE: {
            char* endptr;
            errno = 0;

            // Convert optarg string to a long.
            userInput->rotations = strtol(optarg, &endptr, BASE_10);

            if ((optarg == endptr) || (*endptr != EOS)) {
                return EXIT_INVALID_PARAMETER;
            }

            if (errno == ERANGE) {
                return EXIT_INVALID_PARAMETER;
            }
            break;
        }

            // If valid command supplied, or none supplied at all
        default:
            return EXIT_NO_COMMAND;
        }
    }

    // All options parses successfully
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

int check_long_within_bounds(const long num, const long min, const long max)
{
    // Check number within specified bounds
    if ((num < min) || (num > max)) {
        return -1;
    }

    return EXIT_SUCCESS;
}

long verify_long_arg_with_bounds(
        const char* arg, const long min, const long max)
{
    char* endptr;
    errno = 0;

    // Convert arg string to a long.
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

void glitch_offset_invalid_message(const char* arg)
{
    fputs(glitchUsageMessage, stderr);
    fputs(glitchOffsetValMessage, stderr);
    fprintf(stderr, gotStrMessage, arg);
}

int handle_commands(UserInput* userInput)
{
    if (userInput->help) { // If help mode enabled
        fputs(helpMessage, stdout);
    }

    // An input file is required all non-help commands
    if (!(userInput->input)) {
        return EXIT_MISSING_INPUT_FILE;
    }

    // Check the file type of supplied file path
    if (check_valid_file_type(fileType, userInput->inputFilePath) == -1) {
        return EXIT_INVALID_FILE_TYPE;
    }

    // Initialise struct to store BMP data
    BMP bmpImage;
    initialise_bmp(&bmpImage);
    int status = EXIT_SUCCESS;

    // Goes through command modes, ensuring all memory is freed, and correct
    // exit status is recorded for return value.
    while (1) {
        // Attempt to open the BMP and read file headers
        if ((status = open_bmp(&bmpImage, userInput->inputFilePath))
                != EXIT_SUCCESS) {
            break;
        }

        if (userInput->header) { // If header dump mode enabled
            dump_headers(&bmpImage);
        }

        // Attempt to load pixel data from file into bmpImage struct
        if ((status = handle_bmp_loading(&bmpImage)) != EXIT_SUCCESS) {
            break;
        }

        if (userInput->flip) { // If flip mode enabled
            flip_image(bmpImage.image);
        }

        if (userInput->filters) {
            ColourFilter filter
                    = (ColourFilter)handle_colour_filters(userInput->filters);
            if (filter == NULL) { // FIX check this is even possible
                break; // FIX update status add add error message
            }

            filter(bmpImage.image);
        }

        if (userInput->maxBrightness) {
            brightness_cap_filter(bmpImage.image, userInput->maxBrightness);
        }

        if (userInput->grayscale) { // If grayscale mode enabled
            gray_filter(bmpImage.image);
        }

        if (userInput->glitch) { // If glitch mode enabled
            if (glitch_effect(bmpImage.image, userInput->glitch) == -1) {
                status = EXIT_OUT_OF_BOUNDS; // FIX
                break;
            }
        }

        if (userInput->swap) { // If swap mode enabled
            swap_red_blue(bmpImage.image);
        }

        if (userInput->contrast) { // If contrast mode enabled
            contrast_effect(
                    bmpImage.image, userInput->contrast, 100, 160); // Fix Magic
        }

        if (userInput->dim) { // If dim mode enabled
            dim_effect(bmpImage.image, userInput->dim);
        }

        if (userInput->combine) { // If image combination mode enabled
            if ((status = handle_combine(userInput, &bmpImage))
                    != EXIT_SUCCESS) {
                break;
            }
        }

        if (userInput->invert) { // If image inversion mode enabled
            invert_colours(bmpImage.image);
        }

        if (userInput->average) { // If pixel averaging mode enabled
            average_pixels(bmpImage.image);
        }

        if (userInput->rotations) {
            bmpImage.image = handle_image_rotation(bmpImage.image, userInput->rotations);
            if (bmpImage.image == NULL) {
                break; // FIX add specific error code
            }

            if (userInput->rotations % 2) {
                const int32_t temp = bmpImage.infoHeader.bitmapWidth;
                bmpImage.infoHeader.bitmapWidth
                        = bmpImage.infoHeader.bitmapHeight;
                bmpImage.infoHeader.bitmapHeight = temp;
            }
        }

        if (userInput->output) { // If output file mode enabled
            if (write_bmp_with_header_provided(
                        &bmpImage, userInput->outputFilePath)
                    == -1) {
                status = EXIT_OUTPUT_FILE_ERROR;
                break;
            }
        }

        if (userInput->print) { // If terminal printing mode enabled
            print_image_to_terminal(bmpImage.image);
        }

        break;
    }

    // Cleanup and exit
    free_image_resources(&bmpImage);
    return status;
}

Function handle_colour_filters(const uint8_t filters)
{
    const uint8_t index = filters & 0x07;

    const Function filterMap[8] = {
            NULL,
            (Function)&filter_red,
            (Function)&filter_green,
            (Function)&filter_red_green,
            (Function)&filter_blue,
            (Function)&filter_red_blue,
            (Function)&filter_green_blue,
            (Function)&filter_all,
    };

    return filterMap[index];
}

uint8_t handle_colour_filter_arg_parsing(const char* arg)
{
    const char allowed[] = {'r', 'R', 'g', 'G', 'b', 'B', EOS};
    uint8_t colourBitVect = 0;

    for (size_t i = 0; i < strlen(arg); i++) {

        size_t j = 0;
        while (1) {

            // Character did not match any valid char.
            if (allowed[j] == EOS) {
                fprintf(stderr, invalidFilterColourMessage, arg);
                return 0;
            }

            if ((char)(arg[i]) == allowed[j]) {

                // Sets bit in bit vector to indicate presence of colour to
                // filter. Both 'r' and 'R' are represented by the same bit,
                // hence the division by two.
                colourBitVect |= (1 << (j / 2));
                break;
            }
            j++;
        }
    }

    return colourBitVect;
}

int handle_combine(const UserInput* userInput, BMP* bmpImage)
{
    if (check_valid_file_type(fileType, userInput->combineFilePath) == -1) {
        return EXIT_INVALID_FILE_TYPE;
    }

    // Exit if input and combine file paths match
    if (!strcmp(userInput->inputFilePath, userInput->combineFilePath)) {
        fputs(nonUniquePathsMessage, stderr);
        return EXIT_SAME_FILE;
    }

    BMP combinedImage;
    initialise_bmp(&combinedImage);

    int status = EXIT_SUCCESS;
    while (1) {
        if ((status = open_bmp(&combinedImage, userInput->combineFilePath))
                != EXIT_SUCCESS) {
            break;
        }

        if ((status = handle_bmp_loading(bmpImage)) != EXIT_SUCCESS) {
            break;
        }

        // Flip in upside down to correct for BMP pixel storage format
        flip_image(combinedImage.image);

        if ((status = combine_images(bmpImage->image, combinedImage.image))
                != EXIT_SUCCESS) {
            break;
        }

        break;
    }

    // Cleanup and exit
    free_image_resources(&combinedImage);
    return status;
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

int check_valid_file_type(const char* const type, const char* filePath)
{
    // Check path ends with expected file type string
    if (!ends_with(type, filePath)) {

        // Prints error message to stderr
        fputs(fileTypeMessage, stderr);
        fprintf(stderr, unexpectedArgMessage, filePath, type);
        return -1;
    }

    return EXIT_SUCCESS;
}
