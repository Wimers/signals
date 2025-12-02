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

// Error messages
const char* const fileTypeMessage = "Input file must be \".bmp\"\n";
const char* const fileOpeningErrorMessage
        = "Error opening file \"%s\" for reading.\n";
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
const char* const gotIntMessage = "    Got \"%d\".\n";
const char* const glitchUsageMessage
        = "Glitch Effect\nUsage:  -l, --glitch <offset>  "
          "    - Apply horizontal glitch effect\n";

// Program constant strings
const char* const usageMessage = "Usage: ./bmp <option> [--input <file>] ...\n";
const char* const helpMessage
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
const char* const suXFormat = "%-25s %-15u %X\n";
const char* const sdXFormat = "%-25s %-15d %X\n";
const char* const sssFormat = "%-25s %-15s %s\n";
const char* const ssdFormat = "%-25s %-15s %d\n";
const char* const sudFormat = "%-25s %-15u %d\n";
const char* const fileType = ".bmp";
const char* const optstring = "i:o:dphfb:c:l:gvua"; // Defined program flags

// Assorted constant chars
const char* const readMode = "rb";

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
        {NULL, 0, NULL, 0},
};

void verify_glitch_arg(UserInput* userInput, const char* arg);

int main(const int argc, char** argv)
{
    early_argument_checks(argc, argv);

    UserInput userInput;
    (void)memset(&userInput, 0, sizeof(userInput));

    parse_user_commands(argc, argv, &userInput);
    handle_commands(&userInput);
    exit(EXIT_OK);
}

void check_for_empty_args(const int argc, char** argv)
{
    for (int i = 0; i < argc; i++) {
        if (argv[i][0] == EOS) {
            fputs(emptyArgsMessage, stderr);
            fputs(usageMessage, stderr);
            exit(EXIT_INVALID_ARG); // FIX
        }
    }
}

void check_file_opened(FILE* file, const char* const filePath)
{
    if (file == NULL) { // Check if file can be opened
        fprintf(stderr, fileOpeningErrorMessage, filePath);
        exit(EXIT_FILE_CANNOT_BE_READ);
    }
}

void early_argument_checks(const int argc, char** argv)
{
    if (!(argc >= MIN_CMD_ARGS)) {
        fputs(noArgsProvidedMessage, stderr);
        fputs(usageMessage, stderr);
        fputs(userHelpPrompt, stderr);
        exit(EXIT_INVALID_ARG); // FIX
    }

    check_for_empty_args(argc, argv);
}

void parse_user_commands(const int argc, char** argv, UserInput* userInput)
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
        case BRIGHTNESS_CAP:
            userInput->maxBrightness = (uint8_t)atoi(optarg);
            break;
        case COMBINE:
            userInput->combine = 1;
            userInput->combineFilePath = optarg;
            break;
        case GLITCH:
            verify_glitch_arg(userInput, optarg);
            break;
        case AVE:
            userInput->average = 1;
            break;
        default:
            exit(EXIT_NO_COMMAND);
        }
    }
}

void verify_glitch_arg(UserInput* userInput, const char* arg)
{
    const int len = strlen(arg);

    for (int i = 0; i < len; i++) {
        if (!isdigit(arg[i])) {
            fputs(glitchUsageMessage, stderr);
            fputs(glitchOffsetValMessage, stderr);
            fprintf(stderr, gotStrMessage, arg);
            exit(EXIT_INVALID_ARG); // FIX
        }
    }

    const int32_t glitchOffset = (int32_t)atoi(arg);

    if (glitchOffset) {
        userInput->glitch = glitchOffset;
    } else {
        fputs(glitchUsageMessage, stderr);
        fputs(glitchOffsetValMessage, stderr);
        fprintf(stderr, gotStrMessage, arg);
        exit(EXIT_INVALID_ARG);
    }
}

void handle_commands(UserInput* userInput)
{
    if (userInput->help) {
        fputs(helpMessage, stdout);
    }

    if (!(userInput->input)) {
        return;
    }
    check_valid_file_type(userInput->inputFilePath);

    BMP bmpImage;
    initialise_bmp(&bmpImage);
    open_bmp(&bmpImage, userInput->inputFilePath);

    if (userInput->header) {
        dump_headers(&bmpImage);
    }

    if (!userInput->flip) {
        bmpImage.image = flip_image(bmpImage.image);
    }

    if (userInput->filter) {
        filter_red(bmpImage.image);
    }

    if (userInput->maxBrightness) {
        brightness_cap_filter(bmpImage.image, userInput->maxBrightness);
    }

    if (userInput->grayscale) {
        gray_filter(bmpImage.image);
    }

    if (userInput->glitch) {
        glitch_effect(bmpImage.image, userInput->glitch);
    }

    if (userInput->combine) {
        check_valid_file_type(userInput->inputFilePath);

        BMP combinedImage;
        initialise_bmp(&combinedImage);
        open_bmp(&combinedImage, userInput->combineFilePath);

        combinedImage.image = flip_image(combinedImage.image);
        combine_images(bmpImage.image, combinedImage.image);

        // Free resources and memory
        if (combinedImage.image != NULL) {
            free_image(combinedImage.image);
        }
        fclose(combinedImage.file);
    }

    if (userInput->invert) {
        filter_invert_colours(bmpImage.image);
    }

    if (userInput->average) {
        average_pixels(bmpImage.image);
    }

    if (userInput->output) {
        write_bmp_with_header_provided(&bmpImage, userInput->outputFilePath);
    }

    if (userInput->print) {
        print_image_to_terminal(bmpImage.image);
    }

    if (bmpImage.image != NULL) {
        free_image(bmpImage.image);
    }

    fclose(bmpImage.file);
}

void print_bmp_header(const BmpHeader* bmp)
{
    fprintf(stdout, sssFormat, "BMP Header", "Data", "Hex");
    fputs(lineSeparator, stdout);
    fprintf(stdout, ssdFormat, "ID", (char*)&(bmp->id), bmp->id);
    fprintf(stdout, suXFormat, "Size", bmp->bmpSize, bmp->bmpSize);
    fprintf(stdout, suXFormat, "Offset", bmp->offset, bmp->offset);
}

void print_bmp_info_header(const BmpInfoHeader* bmp)
{
    fputs(newlineStr, stdout);
    printf(sssFormat, "DIB Header", "Data", "Hex");
    fputs(lineSeparator, stdout);
    printf(suXFormat, "Header Size", bmp->headerSize, bmp->headerSize);
    printf(sdXFormat, "Bitmap Width", bmp->bitmapWidth, bmp->bitmapWidth);
    printf(sdXFormat, "Bitmap Height", bmp->bitmapHeight, bmp->bitmapHeight);
    printf(sudFormat, "Num. Colour Planes", bmp->colourPlanes,
            bmp->colourPlanes);
    printf(sudFormat, "Bits Per Pixel", bmp->bitsPerPixel, bmp->bitsPerPixel);
    printf(suXFormat, "Compression", bmp->compression, bmp->compression);
    printf(suXFormat, "Image Size", bmp->imageSize, bmp->imageSize);
    printf(sdXFormat, "Horizontal Resolution", bmp->horzResolution,
            bmp->horzResolution);
    printf(sdXFormat, "Verticle Resolution", bmp->vertResolution,
            bmp->vertResolution);
    printf(suXFormat, "Colours In Palette", bmp->coloursInPalette,
            bmp->coloursInPalette);
    printf(suXFormat, "Important Colours", bmp->importantColours,
            bmp->importantColours);
}

int ends_with(const char* const target, const char* arg)
{
    const int lenArg = strlen(arg);
    const int lenTarget = strlen(target);

    if (lenArg < lenTarget) {
        return -1;
    }

    return !(strcmp(target, &(arg[lenArg - lenTarget])));
}

void check_valid_file_type(const char* filePath)
{
    if (!ends_with(fileType, filePath)) {
        fputs(fileTypeMessage, stderr);
        fprintf(stderr, unexpectedArgMessage, filePath, fileType);
        exit(EXIT_INVALID_ARG);
    }
}
