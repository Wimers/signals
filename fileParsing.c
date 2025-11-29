// Included Libraries
#include "fileParsing.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

const char* const invalidColourPlanesMessage
        = "The number of colour planes must be 1, got \"%d\"\n";
const char* const bitMap = "BM";
const char* const newlineStr = "\n";
const char* const eofAddrMessage = "End of File Addr: %lu\n";
const char* const gradient = " .:-=+#%@";
const char* const colouredBlockFormatter = "\033[38;2;%d;%d;%dm██\033[0m";

// Assorted constant chars
const char newlineChar = '\n';

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

void read_pixel_row(
        FILE* file, Image* image, int rowNumber, uint32_t byteOffset)
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

    // Calculate offset required due to row padding (32-bit DWORD len)
    uint32_t byteOffset
            = calc_row_byte_offset(bmp->bitsPerPixel, bmp->bitmapWidth);

    // Seek to start of pixel data
    fseek(file, header->offset, SEEK_SET);

    // For each row of pixels
    for (int height = 0; height < bmp->bitmapHeight; height++) {
        read_pixel_row(file, image, height, byteOffset);
    }

    // Print offset of file pointer after iterating all pixels
    printf(eofAddrMessage, ftell(file));

    if (ftell(file) != header->bmpSize) {
        free_image(image);
        return NULL;
    }

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
        buffer[bufferPosition++] = newlineChar;
    }

    if (bufferPosition) { // If remaining chars
        fwrite(buffer, 1, bufferPosition, stdout);
    }
}

void get_pixel(const int x, const int y, const BmpHeader* restrict header,
        const BmpInfoHeader* restrict bmp,
        FILE* file) // Highly inefficient
{
    if ((x - 1 > bmp->bitmapWidth) || (x <= 0)) {
        exit(EXIT_OUT_OF_BOUNDS);
    }

    if ((y - 1 > bmp->bitmapHeight) || (y <= 0)) {
        exit(EXIT_OUT_OF_BOUNDS);
    }

    // Initialise parameters
    uint8_t pixel[RGB_PIXEL_BYTE_SIZE];

    // Calculate offset required due to row padding (32-bit DWORD len)
    const uint32_t byteOffset
            = calc_row_byte_offset(bmp->bitsPerPixel, bmp->bitmapWidth);
    const uint32_t pixelOffset = header->offset + ((y * x) - 1) * sizeof(pixel)
            + (y - 1) * byteOffset;

    if (pixelOffset) { // If offset non-zero seek to pixel
        fseek(file, pixelOffset, SEEK_SET);
    }

    read_pixel(&pixel, file);
    fprintf(stdout, "%dx%d -> RGB: (%d, %d, %d)\n", x, y, pixel[0], pixel[1],
            pixel[2]);
}

void read_pixel(uint8_t (*pixel)[RGB_PIXEL_BYTE_SIZE], FILE* file)
{
    // For each colour (RGB)
    for (int colour = 1; colour <= RGB_PIXEL_BYTE_SIZE; colour++) {

        // Reads intensity of colour and stores into pixel
        fread(&((*pixel)[RGB_PIXEL_BYTE_SIZE - colour]), 1, 1, file);
    }
}

uint32_t calc_row_byte_offset(const int bitsPerPixel, const int bitmapWidth)
{
    // Calculate offset required due to row padding (32-bit DWORD len)
    uint32_t byteOffset
            = (((bitsPerPixel * bitmapWidth) % BMP_ROW_DWORD_LEN) / SIZE_BYTE);
    return byteOffset;
}

Image* create_image(uint32_t width, uint32_t height)
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
