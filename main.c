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

int main(const int argc, char** argv)
{
    early_argument_checks(argc, argv);

    Flag opt;
    int help = 0;
    int header = 0;
    int print = 0;
    int input = 0;
    int filter = 0;
    int grayscale = 0;
    int invert = 0;
    int flip = 0;

    UserInput userInput;
    memset(&userInput, 0, sizeof(userInput));

    // loop over all of the options
    while ((opt = getopt_long(argc, argv, optstring, long_options, NULL))
            != -1) {
        switch (opt) {
        case HELP:
            help = 1;
            break;
        case DUMP_HEADER:
            header = 1;
            break;
        case PRINT_IMAGE:
            print = 1;
            break;
        case INPUT_FILE:
            input = 1;
            userInput.inputFilePath = optarg;
            break;
        case OUTPUT_FILE:
            userInput.outputFilePath = optarg;
            break;
        case FILTERS:
            filter = 1;
            userInput.filters = optarg;
            break;
        case GRAY_SCALE:
            grayscale = 1;
            break;
        case INVERT:
            invert = 1;
            break;
        case FLIP:
            flip = 1;
            break;
        default:
            exit(EXIT_NO_COMMAND);
        }
    }

    const char* filePath = userInput.inputFilePath;

    if (!ends_with(fileType, filePath)) {
        fputs(fileTypeMessage, stderr);
        exit(EXIT_INVALID_ARG);
    }

    FILE* file = fopen(filePath, readMode);
    check_file_opened(file, filePath);

    BmpHeader bmp;
    BmpInfoHeader infoHeader;
    read_headers(&bmp, &infoHeader, file);

    Image* image = NULL;

    if (help) {
        fputs(usageMessage, stdout);
    }
    if (header) {
        dump_headers(&bmp, &infoHeader);
    }
    if (input) {
        image = load_bmp_2d(file, &bmp, &infoHeader);

        if (image == NULL) {
            fputs(fileOpeningErrorMessage, stderr);
            fclose(file);
            exit(EXIT_FILE_INTEGRITY);
        }
    }

    if (!flip) {
        image = flip_image(image);
    }

    if (filter) {
        filter_red(image);
    }

    if (grayscale) {
        gray_filter(image);
    }

    if (invert) {
        filter_invert_colours(image);
    }

    if (print) {
        print_image_to_terminal(image);
    }
    if (image != NULL) {
        free_image(image);
    }

    fclose(file);
    exit(EXIT_OK);
}

void check_for_empty_args(const int argc, char** argv)
{
    for (int i = 0; i < argc; i++) {
        if (argv[i][0] == EOS) {
            exit(EXIT_INVALID_ARG);
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
    if (!(argc >= 2)) {
        fputs(invalidArgsMessage, stderr);
        exit(EXIT_INVALID_ARG);
    }

    check_for_empty_args(argc, argv);
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
    int lenArg;
    int lenTarget;
    if ((lenArg = strlen(arg)) < (lenTarget = strlen(target))) {
        return -1;
    }

    return !(strcmp(target, &(arg[lenArg - lenTarget])));
}
