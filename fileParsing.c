// Included Libraries
#include "fileParsing.h"
#include "main.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// Error message
const char* const invalidColourPlanesMessage
        = "The number of colour planes must be 1, got \"%d\"\n";

// Constant program strings
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

void free_image(Image* image)
{
    free((image->pixels)[0]);
    free((void*)image->pixels);
    free(image);
    image = NULL;
}

void free_image_resources(BMP* bmpImage)
{
    // Safely free allocated memory for storing pixel data
    if (bmpImage->image != NULL) {
        free_image(bmpImage->image);
    }

    // Safely close the BMP image file stream
    if (bmpImage->file != NULL) {
        fclose(bmpImage->file);
        bmpImage->file = NULL;
    }
}

int open_bmp(BMP* bmpImage, const char* const filePath)
{
    bmpImage->file = fopen(filePath, readMode);

    if (check_file_opened(bmpImage->file, filePath) == -1) {
        return EXIT_FILE_CANNOT_BE_READ;
    }

    if (read_headers(bmpImage) == -1) {
        return EXIT_FILE_INTEGRITY;
    }

    bmpImage->image = load_bmp_2d(
            bmpImage->file, &(bmpImage->bmpHeader), &(bmpImage->infoHeader));

    if (bmpImage->image == NULL) {
        fputs(fileOpeningErrorMessage, stderr);
        fclose(bmpImage->file);
        bmpImage->file = NULL;
        return EXIT_FILE_INTEGRITY;
    }

    return EXIT_SUCCESS;
}

int read_headers(BMP* bmpImage)
{
    parse_bmp_header(bmpImage);

    if (parse_bmp_info_header(bmpImage) == -1) {
        return -1;
    }

    return EXIT_SUCCESS;
}

void dump_headers(const BMP* bmpImage)
{
    print_bmp_header(&(bmpImage->bmpHeader));
    print_bmp_info_header(&(bmpImage->infoHeader));
}

void parse_bmp_header(BMP* bmpImage)
{
    FILE* file = bmpImage->file;
    BmpHeader* bmpHeader = &(bmpImage->bmpHeader);

    // Store the value in the ID field
    fread(&(bmpHeader->id), sizeof(uint16_t), 1, file);

    // Store the size of the BMP file
    fread(&(bmpHeader->bmpSize), sizeof(uint32_t), 1, file);

    // Junk can be ignored, however, we will store incase it is useful later
    fread(&(bmpHeader->junk), sizeof(uint32_t), 1, file);

    // Store the pixel array offset value
    fread(&(bmpHeader->offset), sizeof(uint32_t), 1, file);
}

int parse_bmp_info_header(BMP* bmpImage)
{
    FILE* file = bmpImage->file;
    BmpInfoHeader* info = &(bmpImage->infoHeader);

    // Seek to start of header
    fseek(file, BITMAP_FILE_HEADER_SIZE, SEEK_SET);

    fread(&info->headerSize, sizeof(uint32_t), 1, file); // Header size in bytes
    fread(&info->bitmapWidth, sizeof(int32_t), 1,
            file); // Bitmap width in pixles
    fread(&info->bitmapHeight, sizeof(int32_t), 1,
            file); // Bitmap height in pixels
    fread(&info->colourPlanes, sizeof(uint16_t), 1,
            file); // Number of colour planes

    if ((int)(info->colourPlanes) != 1) { // Must be one
        fprintf(stderr, invalidColourPlanesMessage, (int)(info->colourPlanes));
        return -1;
    }

    fread(&info->bitsPerPixel, sizeof(uint16_t), 1,
            file); // Image colour depth (i.e. 16, 24)
    fread(&info->compression, sizeof(uint32_t), 1,
            file); // BI_RGB (no compression) most common
    fread(&info->imageSize, sizeof(uint32_t), 1,
            file); // Size of the raw bitmap data
    fread(&info->horzResolution, sizeof(uint32_t), 1, file);
    fread(&info->vertResolution, sizeof(uint32_t), 1, file);
    fread(&info->coloursInPalette, sizeof(uint32_t), 1, file);
    fread(&info->importantColours, sizeof(uint32_t), 1,
            file); // 0 if all colours important

    return EXIT_SUCCESS;
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
        fread(&((*pixel)[RGB_PIXEL_BYTE_SIZE - colour]), sizeof(uint8_t), 1,
                file);
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

void write_bmp_with_header_provided(BMP* bmpImage, const char* filename)
{
    BmpHeader* bmpHeader = &(bmpImage->bmpHeader);
    BmpInfoHeader* infoHeader = &(bmpImage->infoHeader);
    Image* image = bmpImage->image;

    FILE* output = fopen(filename, "wb");

    // Write BmpHeader
    fwrite(&bmpHeader->id, sizeof(uint16_t), 1, output);
    fwrite(&bmpHeader->bmpSize, sizeof(uint32_t), 1, output);
    fwrite(&bmpHeader->junk, sizeof(uint32_t), 1, output);
    fwrite(&bmpHeader->offset, sizeof(uint32_t), 1, output);

    // Write BmpInfoHeader
    fwrite(&infoHeader->headerSize, sizeof(uint32_t), 1, output);
    fwrite(&infoHeader->bitmapWidth, sizeof(int32_t), 1, output);
    fwrite(&infoHeader->bitmapHeight, sizeof(int32_t), 1, output);
    fwrite(&infoHeader->colourPlanes, sizeof(uint16_t), 1, output);
    fwrite(&infoHeader->bitsPerPixel, sizeof(uint16_t), 1, output);
    fwrite(&infoHeader->compression, sizeof(uint32_t), 1, output);
    fwrite(&infoHeader->imageSize, sizeof(uint32_t), 1, output);
    fwrite(&infoHeader->horzResolution, sizeof(uint32_t), 1, output);
    fwrite(&infoHeader->vertResolution, sizeof(uint32_t), 1, output);
    fwrite(&infoHeader->coloursInPalette, sizeof(uint32_t), 1, output);
    fwrite(&infoHeader->importantColours, sizeof(uint32_t), 1, output);

    const uint32_t currentPosition = ftell(output);
    const uint32_t gapSize = bmpHeader->offset - currentPosition;
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

int check_file_opened(FILE* file, const char* const filePath)
{
    if (file == NULL) { // Check if file can be opened
        fprintf(stderr, fileOpeningErrorMessage, filePath);
        return -1;
    }

    return EXIT_SUCCESS;
}
