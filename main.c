// Included Libraries
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fileParsing.h"

int main(int argc, char** argv)
{
    if (argc != 2) {
        fputs(invalidArgsMessage, stderr);
        exit(EXIT_INVALID_ARG);
    }

    const char* filePath = argv[FILE_PATH_IX];
    if (filePath[0] == EOS) {
        exit(EXIT_INVALID_ARG);
    }

    FILE* file = fopen(filePath, readMode);
    if (file == NULL) { // Check if file can be opened
        fprintf(stderr, fileOpeningErrorMessage, filePath);
        exit(EXIT_FILE_CANNOT_BE_READ);
    }

    BmpHeader bmp;
    memset(&bmp, 0, sizeof(bmp));
    parse_bmp_header(&bmp, file);

    BmpInfoHeader infoHeader;
    memset(&infoHeader, 0, sizeof(infoHeader));
    parse_bmp_info_header(&infoHeader, file);

    print_bmp_header(bmp);
    print_bmp_info_header(infoHeader);

    fclose(file);

    exit(EXIT_OK);
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

    fread(&bmp->bitsPerPixel, 2, 1, file); // Image colour depth (i.e. 16, 24)
    fread(&bmp->compression, 4, 1, file); // BI_RGB (no compression) most common
    fread(&bmp->imageSize, 4, 1, file); // Size of the raw bitmap data
    fread(&bmp->horzResolution, 4, 1, file);
    fread(&bmp->vertResolution, 4, 1, file);
    fread(&bmp->coloursInPalette, 4, 1, file);
    fread(&bmp->importantColours, 4, 1, file); // 0 if all colours important
}

void print_bmp_header(BmpHeader bmp)
{
    fprintf(stdout, sssFormat, "BMP Header", "Data", "Hex");
    fputs(lineSeparator, stdout);
    fprintf(stdout, ssdFormat, "ID", (char*)&(bmp.id), bmp.id);
    fprintf(stdout, suXFormat, "Size", bmp.bmpSize, bmp.bmpSize);
    fprintf(stdout, suXFormat, "Offset", bmp.offset, bmp.offset);
}

void print_bmp_info_header(BmpInfoHeader bmp)
{
    fputs(newlineStr, stdout);
    printf(sssFormat, "DIB Header", "Data", "Hex");
    fputs(lineSeparator, stdout);
    printf(suXFormat, "Header Size", bmp.headerSize, bmp.headerSize);
    printf(sdXFormat, "Bitmap Width", bmp.bitmapWidth, bmp.bitmapWidth);
    printf(sdXFormat, "Bitmap Height", bmp.bitmapHeight, bmp.bitmapHeight);
    printf(sudFormat, "Num. Colour Planes", bmp.colourPlanes, bmp.colourPlanes);
    printf(sudFormat, "Bits Per Pixel", bmp.bitsPerPixel, bmp.bitsPerPixel);
    printf(suXFormat, "Compression", bmp.compression, bmp.compression);
    printf(suXFormat, "Image Size", bmp.imageSize, bmp.imageSize);
    printf(sdXFormat, "Horizontal Resolution", bmp.horzResolution,
            bmp.horzResolution);
    printf(sdXFormat, "Verticle Resolution", bmp.vertResolution,
            bmp.vertResolution);
    printf(suXFormat, "Colours In Palette", bmp.coloursInPalette,
            bmp.coloursInPalette);
    printf(suXFormat, "Important Colours", bmp.importantColours,
            bmp.importantColours);
}
