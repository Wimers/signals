// Included Libraries
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "fileParsing.h"

int main(const int argc, char** argv)
{
    early_argument_checks(argc, argv);

    Flag opt;
    int help = 0;
    int header = 0;
    int display = 0;

    while ((opt = getopt(argc, argv, optstring)) != -1) {
        switch (opt) {
        case HELP:
            help = 1;
            break;
        case HEADER_DUMP:
            header = 1;
            break;
        case DISPLAY_IMAGE:
            display = 1;
            break;
        default:
            exit(EXIT_NO_COMMAND);
        }
    }

    if (optind >= argc) {
        fputs(invalidArgsMessage, stderr);
        exit(EXIT_INVALID_ARG);
    }

    const char* filePath = argv[optind];

    FILE* file = fopen(filePath, readMode);
    check_file_opened(file, filePath);

    BmpHeader bmp;
    BmpInfoHeader infoHeader;
    read_headers(&bmp, &infoHeader, file);

    if (help) {
        printf(usageMessage);
    }
    if (header) {
        dump_headers(&bmp, &infoHeader);
    }
    if (display) {
        display_image(&bmp, &infoHeader, file);
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

void read_headers(
        BmpHeader* restrict bmp, BmpInfoHeader* restrict infoHeader, FILE* file)
{
    memset(bmp, 0, sizeof(*bmp));
    parse_bmp_header(bmp, file);

    memset(infoHeader, 0, sizeof(*infoHeader));
    parse_bmp_info_header(infoHeader, file);
}

void dump_headers(const BmpHeader* bmp, const BmpInfoHeader* infoHeader)
{
    print_bmp_header(bmp);
    print_bmp_info_header(infoHeader);
}

void parse_bmp_header(BmpHeader* bmp, FILE* file)
{
    // Store the value in the ID field
    fread(&(bmp->id), 2, 1, file);

    // Store the size of the BMP file
    fread(&(bmp->bmpSize), 4, 1, file);

    // Jump to pixle array offset, and store value
    fseek(file, 0x0A, SEEK_SET);
    fread(&(bmp->offset), 4, 1, file);
}

void parse_bmp_info_header(BmpInfoHeader* bmp, FILE* file)
{
    fseek(file, 14, SEEK_SET); // Seek to start of header

    fread(&bmp->headerSize, 4, 1, file); // Header size in bytes
    fread(&bmp->bitmapWidth, 4, 1, file); // Bitmap width in pixles
    fread(&bmp->bitmapHeight, 4, 1, file); // Bitmap height in pixels
    fread(&bmp->colourPlanes, 2, 1, file); // Number of colour planes

    if ((int)(bmp->colourPlanes) != 1) { // Must be one
        fprintf(stderr, invalidColourPlanesMessage, (int)(bmp->colourPlanes));
        exit(EXIT_FILE_INTEGRITY);
    }

    fread(&bmp->bitsPerPixel, 2, 1,
            file); // Image colour depth (i.e. 16, 24)
    fread(&bmp->compression, 4, 1,
            file); // BI_RGB (no compression) most common
    fread(&bmp->imageSize, 4, 1, file); // Size of the raw bitmap data
    fread(&bmp->horzResolution, 4, 1, file);
    fread(&bmp->vertResolution, 4, 1, file);
    fread(&bmp->coloursInPalette, 4, 1, file);
    fread(&bmp->importantColours, 4, 1, file); // 0 if all colours important
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

void display_image(const BmpHeader* restrict header,
        const BmpInfoHeader* restrict bmp, FILE* file)
{
    // Initialise parameters
    uint8_t pixel[RGB_PIXEL_BYTE_SIZE];
    int width;
    int height = 0;
    uint32_t byteOffset;

    // Seek to start of pixel data
    fseek(file, header->offset, SEEK_SET);

    // For each row of pixels
    for (; height < bmp->bitmapHeight; height++) {
        width = 0;

        // For each pixel in row
        for (; width < bmp->bitmapWidth; width++) {
            byteOffset = 0;

            // For each colour (RGB)
            for (int colour = 0; colour < RGB_PIXEL_BYTE_SIZE; colour++) {

                // Reads intensity of colour
                fread(&pixel[colour], 1, 1, file);
            }

            // Skip pixels out of range, or out of resolution context
            if (!(width > MAX_TERMINAL_ASCII_WIDTH)
                    && !(height % VERT_TERMINAL_RESOLUTION)) {
                // Printf block character to terminal with the colour of the
                // pixel read
                printf("\033[38;2;%d;%d;%dm██\033[0m", pixel[2], pixel[1],
                        pixel[0]);
            }
        }

        // Calculate offset required due to row padding (32-bit DWORD len)
        byteOffset
                = (((bmp->bitsPerPixel * bmp->bitmapWidth) % BMP_ROW_DWORD_LEN)
                        / SIZE_BYTE);

        if (byteOffset) { // If offset non-zero update file pointer
            fseek(file, byteOffset, SEEK_CUR);
        }

        if (!(height % VERT_TERMINAL_RESOLUTION)) {
            printf(newlineStr); // Newline terminates each row of pixels
        }
    }

    // Prints offset of file pointer after iterating all pixels
    printf(eofAddrMessage, ftell(file));
}

void brightness_gradient_mapping(const int brightness)
{
    const int index = brightness / BMP_ROW_DWORD_LEN;

    if ((0 <= index) && (index < (int)(strlen(gradient)))) {
        const char symbol = gradient[index];
        fputc(symbol, stdout);
        return;
    }

    exit(EXIT_FILE_PARSING_ERROR);
}
