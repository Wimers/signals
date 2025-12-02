// Included Libraries
#include "fileParsing.h"
#include "main.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

const char* const invalidColourPlanesMessage
        = "The number of colour planes must be 1, got \"%d\"\n";
const char* const bitMap = "BM";
const char* const eofAddrMessage = "End of File Addr: %lu\n";
const char* const colouredBlockFormatter = "\033[38;2;%d;%d;%dm██\033[0m";
const char* const newlineStr = "\n";

// Assorted constant chars
const char newlineChar = '\n';

void initialise_bmp(BMP* bmpImage)
{
    BmpHeader header;
    BmpInfoHeader infoHeader;

    // Initialise memory to zero's
    memset(bmpImage, 0, sizeof(*bmpImage));
    memset(&header, 0, sizeof(header));
    memset(&infoHeader, 0, sizeof(infoHeader));

    // Assign to the bmpImage
    bmpImage->bmpHeader = header;
    bmpImage->infoHeader = infoHeader;
}

void open_bmp(BMP* bmpImage, const char* filePath)
{
    bmpImage->file = fopen(filePath, readMode);
    if (bmpImage->file == NULL) {
        exit(100);
    }
    check_file_opened(bmpImage->file, filePath);
    read_headers(bmpImage);

    bmpImage->image = load_bmp_2d(
            bmpImage->file, &(bmpImage->bmpHeader), &(bmpImage->infoHeader));

    if (bmpImage->image == NULL) {
        fputs(fileOpeningErrorMessage, stderr);
        fclose(bmpImage->file);
        exit(EXIT_FILE_INTEGRITY);
    }
}

void read_headers(BMP* bmpImage)
{
    parse_bmp_header(&(bmpImage->bmpHeader), bmpImage->file);
    parse_bmp_info_header(&(bmpImage->infoHeader), bmpImage->file);
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

void read_pixel_row(FILE* file, Image* image, const int rowNumber,
        const uint32_t byteOffset)
{
    // Read row of pixels
    fread((image->pixels)[rowNumber], sizeof(Pixel), image->width, file);

    if (byteOffset) { // If offset non-zero update file pointer
        fseek(file, byteOffset, SEEK_CUR);
    }
}

Image* load_bmp_2d(FILE* file, const BmpHeader* restrict header,
        const BmpInfoHeader* restrict bmp)
{
    // Initialise pixel array
    Image* image = create_image(bmp->bitmapWidth, bmp->bitmapHeight);

    // Calculate offset required due to row padding (32-bit DWORD length)
    const uint32_t byteOffset
            = calc_row_byte_offset(bmp->bitsPerPixel, bmp->bitmapWidth);

    // Seek to start of pixel data
    fseek(file, header->offset, SEEK_SET);

    // For each row of pixels
    for (int height = 0; height < bmp->bitmapHeight; height++) {
        read_pixel_row(file, image, height, byteOffset);
    }

    // Print offset of file pointer after iterating all pixels
    printf(eofAddrMessage, ftell(file));
    return image;
}

void print_image_to_terminal(const Image* image)
{
    // Initialise
    char buffer[OUTPUT_BUFFER_CAPACITY];
    int bufferPosition = 0;

    // For each pixel (RGB)
    for (int height = 0; height < image->height; height++) {
        for (int width = 0; width < image->width; width++) {

            // If buffer does not have room for pixel, write buffer to terminal
            // and reset buffer position.
            if ((bufferPosition + MAX_ANSI_PIXEL_LEN)
                    >= OUTPUT_BUFFER_CAPACITY) {
                fwrite(buffer, 1, bufferPosition, stdout);
                bufferPosition = 0;
            }
            Pixel p = (image->pixels)[height][width];

            // Append pixel to buffer
            bufferPosition += sprintf(&buffer[bufferPosition],
                    colouredBlockFormatter, p.red, p.green, p.blue);
        }

        // If buffer full, write to terminal, and newline terminate
        if (bufferPosition == OUTPUT_BUFFER_CAPACITY) {
            fwrite(buffer, 1, bufferPosition, stdout);
            bufferPosition = 0;
        }
        // Add newline
        buffer[bufferPosition++] = newlineChar;
    }

    if (bufferPosition) { // Output any remaining characters
        fwrite(buffer, 1, bufferPosition, stdout);
    }
}

void read_pixel(uint8_t (*pixel)[RGB_PIXEL_BYTE_SIZE], FILE* file)
{
    // For each colour (RGB)
    for (int colour = 1; colour <= RGB_PIXEL_BYTE_SIZE; colour++) {

        // Reads intensity of colour and stores into pixel
        fread(&((*pixel)[RGB_PIXEL_BYTE_SIZE - colour]), 1, 1, file);
    }
}

uint32_t calc_row_byte_offset(const int bitsPerPixel, const int32_t bitmapWidth)
{
    // Calculate offset required due to row padding (32-bit DWORD len)
    const uint32_t byteOffset
            = (((bitsPerPixel * bitmapWidth) % BMP_ROW_DWORD_LEN) / SIZE_BYTE);
    return byteOffset;
}

Image* create_image(const int32_t width, const int32_t height)
{
    Image* img = malloc(sizeof(Image));
    img->width = width;
    img->height = height;

    // Allocate memory for array of row pointers
    img->pixels = (Pixel**)malloc(height * sizeof(Pixel*));

    // Allocate memory for all pixel data
    Pixel* data = malloc(width * height * sizeof(Pixel));

    // Link the pixel data to each row
    for (int i = 0; i < (int)height; i++) {
        (img->pixels)[i] = &data[width * i];
    }

    return img;
}

void free_image(Image* image)
{
    free((image->pixels)[0]);
    free((void*)image->pixels);
    free(image);
    image = NULL;
}

Image* flip_image(Image* image)
{
    Image* rotatedImage = create_image(image->width, image->height);

    for (int row = 0; row < image->height; row++) {
        int sourceRow = image->height - 1 - row;
        memcpy(rotatedImage->pixels[row], image->pixels[sourceRow],
                image->width * sizeof(Pixel));
    }

    free_image(image);
    return rotatedImage;
}

void write_bmp_with_header_provided(BmpHeader* bmp, BmpInfoHeader* infoHeader,
        Image* image, const char* filename)
{
    FILE* output = fopen(filename, "wb");

    // Write BmpHeader
    fwrite(&bmp->id, 2, 1, output); // 2 bytes
    fwrite(&bmp->bmpSize, 4, 1, output); // 4 bytes
    fwrite(&bmp->junk, 4, 1, output); // 4 bytes
    fwrite(&bmp->offset, 4, 1, output); // 4 bytes

    // Write BmpInfoHeader
    fwrite(&infoHeader->headerSize, 4, 1, output);
    fwrite(&infoHeader->bitmapWidth, 4, 1, output);
    fwrite(&infoHeader->bitmapHeight, 4, 1, output);
    fwrite(&infoHeader->colourPlanes, 2, 1, output);
    fwrite(&infoHeader->bitsPerPixel, 2, 1, output);
    fwrite(&infoHeader->compression, 4, 1, output);
    fwrite(&infoHeader->imageSize, 4, 1, output);
    fwrite(&infoHeader->horzResolution, 4, 1, output);
    fwrite(&infoHeader->vertResolution, 4, 1, output);
    fwrite(&infoHeader->coloursInPalette, 4, 1, output);
    fwrite(&infoHeader->importantColours, 4, 1, output);

    const uint32_t currentPosition = ftell(output);
    const uint32_t gapSize = bmp->offset - currentPosition;
    const uint8_t zero = 0;
    fwrite(&zero, sizeof(zero), gapSize, output);

    const uint32_t byteOffset = calc_row_byte_offset(
            infoHeader->bitsPerPixel, infoHeader->bitmapWidth);

    for (int row = 0; row < infoHeader->bitmapHeight; row++) {
        fwrite(image->pixels[row], infoHeader->bitmapWidth * sizeof(Pixel), 1,
                output);

        if (byteOffset) {
            fwrite(&zero, sizeof(zero), byteOffset, output);
        }
    }

    fclose(output);
}
