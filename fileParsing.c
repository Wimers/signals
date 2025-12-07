// Included Libraries
#include "fileParsing.h"
#include "main.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Error messages
const char* const invalidColourPlanesMessage
        = "The number of colour planes must be 1, got \"%d\".\n";
const char* const fileOpeningErrorMessage
        = "Error opening file \"%s\" for reading.\n";
const char* const errorReadingHeaderMessage = "Error reading \"%s\".\n";
const char* const errorReadingPixelsMessage
        = "Error reading pixels: (row %d)\n";
const char* const negativeWidthMessage = "Bitmap width cannot be negative!\n";
const char* const bmpLoadFailMessage = "BMP could not be loaded.\n";
const char* const headerReadFailMessage
        = "The header from \"%s\" could not be read.\n";
const char* const eofMismatchMessage
        = "Location of EOF mismatch: Got \'%ld\', expected \'%d\'.\n";
const char* const invalidCompressionMessage
        = "Invalid compression method \"%u\", expected result between 0 <-> 13 "
          "(inclusive).\n";
const char* const unsupportedCompressionMessage
        = "Compression method not supported (code: \"%u\").\n";
const char* const invalidBmpTypeMessage
        = "Invalid compression method \"%.2s\".\n";
const char* const unsupportedBmpTypeMessage
        = "Unsupported BMP format \"%.2s\".\n";
const char* const fileSizeCompareMessage
        = "File contains \"%ld\" bytes: metadata asserts \"%u\" "
          "bytes.\n";
const char* const fileTooSmallMessage
        = "File size is too small, %ld less bytes than expected.\n";
const char* const fileCorruptionMessage
        = "File may be corrupted, or contain hidden data (%ld bytes).\n";
const char* const pixelOffsetInvalidMessage = "Pixel data offset invalid.\n";

// Constant program strings
const char* const windowsBmpID = "BM";
const char* const eofAddrMessage = "End of File Addr: %ld\n";
const char* const colouredBlockFormatter = "\033[38;2;%d;%d;%dm██\033[0m";
const char* const newlineStr = "\n";

// Assorted constant chars
const char newlineChar = '\n';

static const char* const BmpIdentifier[]
        = {"BM", "BA", "CI", "CP", "IC", "PT", NULL};

int verify_header_offsets_and_size(BMP* bmpImage);

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

void free_image(Image** image)
{
    if ((image == NULL) || (*image == NULL)) {
        return;
    }

    if ((*image)->pixelData != NULL) {
        free((*image)->pixelData);
        (*image)->pixelData = NULL;
    }

    if ((*image)->pixels != NULL) {
        free((void*)(*image)->pixels);
        (*image)->pixels = NULL;
    }

    free(*image);
    *image = NULL;
}

void safely_close_file(FILE* file)
{
    if (file != NULL) {
        fclose(file);
        file = NULL;
    }
}

void free_image_resources(BMP* bmpImage)
{
    // Safely free allocated memory for storing pixel data
    if (bmpImage->image != NULL) {
        free_image(&(bmpImage->image));
    }

    // Safely close the BMP image file stream
    safely_close_file(bmpImage->file);
}

int open_bmp(BMP* bmpImage, const char* const filePath)
{
    bmpImage->file = fopen(filePath, readMode);

    if (check_file_opened(bmpImage->file, filePath) == -1) {
        return EXIT_FILE_CANNOT_BE_READ;
    }

    if (read_headers(bmpImage) == -1) {
        fprintf(stderr, headerReadFailMessage, filePath);
        return EXIT_FILE_INTEGRITY;
    }

    if (verify_header_offsets_and_size(bmpImage) == -1) {
        return -1; // FIX (add relevent code)
    }

    bmpImage->image = load_bmp_2d(
            bmpImage->file, &(bmpImage->bmpHeader), &(bmpImage->infoHeader));

    if (bmpImage->image == NULL) {
        fprintf(stderr, fileOpeningErrorMessage, filePath);
        safely_close_file(bmpImage->file);
        return EXIT_FILE_INTEGRITY;
    }

    return EXIT_SUCCESS;
}

int read_headers(BMP* bmpImage)
{
    if (parse_bmp_header(bmpImage) == -1) {
        return -1;
    }

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

int is_str_in_const_str_array(const void* restrict arg,
        const char* const strArray[], const size_t nread)
{
    // Initialise
    const int breakPoint = 1000;
    int i = 0;

    // For each string in string array
    while (strArray[i] != NULL) {

        // Check if arg is equal to this string
        if ((strlen(strArray[i]) >= nread)
                && (!memcmp(arg, strArray[i], nread))) {

            return EXIT_SUCCESS; // Arg found
        }

        if (++i > breakPoint) {
            return -1;
        }
    }

    // Arg is not an element of the string array
    return -1;
}

int parse_bmp_header(BMP* bmpImage)
{
    FILE* file = bmpImage->file;
    BmpHeader* bmpHeader = &(bmpImage->bmpHeader);

    // Store the value in the ID field
    READ_HEADER_SAFE(&(bmpHeader->id), sizeof(bmpHeader->id), file, "ID");
    if (is_str_in_const_str_array(
                (char*)&(bmpHeader->id), BmpIdentifier, sizeof(bmpHeader->id))
            == -1) {
        fprintf(stderr, invalidBmpTypeMessage, (char*)&(bmpHeader->id));
        return -1;
    }

    // Check if ID corresponds to an unsupported BMP version
    if (memcmp(&(bmpHeader->id), windowsBmpID, sizeof(bmpHeader->id))) {
        fprintf(stderr, unsupportedBmpTypeMessage, (char*)&(bmpHeader->id));
        return -1;
    }

    // Store the size of the BMP file (needs to be verified later)
    READ_HEADER_SAFE(&(bmpHeader->bmpSize), sizeof(bmpHeader->bmpSize), file,
            "Bitmap Size");

    // Not used, however store for reference
    READ_HEADER_SAFE(&(bmpHeader->reserved1), sizeof(bmpHeader->reserved1),
            file, "Reserved1");

    // Also not used
    READ_HEADER_SAFE(&(bmpHeader->reserved2), sizeof(bmpHeader->reserved2),
            file, "Reserved2");

    // Store the pixel array offset value (needs to be verified later)
    READ_HEADER_SAFE(
            &(bmpHeader->offset), sizeof(bmpHeader->offset), file, "Offset");

    return EXIT_SUCCESS;
}

int parse_bmp_info_header(BMP* bmpImage)
{
    FILE* file = bmpImage->file;
    BmpInfoHeader* info = &(bmpImage->infoHeader);

    // Seek to start of header
    fseek(file, BMP_HEADER_SIZE, SEEK_SET);

    // Header size in bytes
    READ_HEADER_SAFE(
            &(info->headerSize), sizeof(info->headerSize), file, "Header size");

    // Bitmap width in pixles
    READ_HEADER_SAFE(&(info->bitmapWidth), sizeof(info->bitmapWidth), file,
            "Bitmap Width");
    if (info->bitmapWidth < 0) {
        fputs(negativeWidthMessage, stderr);
        fprintf(stderr, gotIntMessage,
                (int)info->bitmapWidth); // Check cast
        return -1;
    }

    // Bitmap height in pixles
    READ_HEADER_SAFE(&(info->bitmapHeight), sizeof(info->bitmapHeight), file,
            "Bitmap Height");

    // Number of colour planes
    READ_HEADER_SAFE(&(info->colourPlanes), sizeof(info->colourPlanes), file,
            "Colour planes");
    if (info->colourPlanes != 1) { // Must be one       // CHECK this cast
        fprintf(stderr, invalidColourPlanesMessage, (int)(info->colourPlanes));
        return -1;
    }

    // Image colour depth (i.e. 16, 24)
    READ_HEADER_SAFE(&(info->bitsPerPixel), sizeof(info->bitsPerPixel), file,
            "Bits per pixel");

    // BI_RGB (no compression) most common
    READ_HEADER_SAFE(&(info->compression), sizeof(info->compression), file,
            "Compression");
    if (info->compression > COMP_METH_VAL_MAX) {
        fprintf(stderr, invalidCompressionMessage, info->compression);
        return -1;
    }
    if (info->compression != BI_RGB) { // Could be updated in future
        fprintf(stderr, unsupportedCompressionMessage, info->compression);
        return -1;
    }

    // Size of the raw bitmap data
    READ_HEADER_SAFE(
            &(info->imageSize), sizeof(info->imageSize), file, "Image size");

    READ_HEADER_SAFE(&(info->horzResolution), sizeof(info->horzResolution),
            file, "Horizontal resolution");
    READ_HEADER_SAFE(&(info->vertResolution), sizeof(info->vertResolution),
            file, "Vertical resolution");
    READ_HEADER_SAFE(&(info->coloursInPalette), sizeof(info->coloursInPalette),
            file, "Colours in palette");

    // 0 if all colours important
    READ_HEADER_SAFE(&(info->importantColours), sizeof(info->importantColours),
            file, "Important colours");

    return EXIT_SUCCESS;
}

int verify_header_offsets_and_size(BMP* bmpImage)
{
    const uint32_t metaFileSize = (bmpImage->bmpHeader).bmpSize;

    // Seek to EOF and store offset
    fseek(bmpImage->file, 0L, SEEK_END);
    const long eofPos = ftell(bmpImage->file);

    if (eofPos < 0) {
        perror("ftell failed");
    }

    const long diff = metaFileSize - eofPos;

    if (diff > 0) {
        fprintf(stderr, fileSizeCompareMessage, eofPos, metaFileSize);
        fprintf(stderr, fileTooSmallMessage, diff);
        return -1;
    }

    if (diff < 0) {
        fprintf(stderr, fileSizeCompareMessage, eofPos, metaFileSize);
        fprintf(stderr, fileCorruptionMessage, -diff);
        return -1;
    }

    const BmpInfoHeader* infoHeader = &(bmpImage->infoHeader);
    const uint32_t offset = (bmpImage->bmpHeader).offset;
    const size_t padding = calc_row_byte_offset(
            infoHeader->bitsPerPixel, (size_t)infoHeader->bitmapWidth);
    const size_t minRequiredBytes
            = ((size_t)abs(infoHeader->bitmapWidth) + padding)
            * (size_t)abs(infoHeader->bitmapHeight);

    if ((offset + minRequiredBytes > (size_t)eofPos)
            || (offset <= (BMP_HEADER_SIZE + DIB_HEADER_SIZE))) {
        fputs(pixelOffsetInvalidMessage, stderr);
        return -1;
    }

    return EXIT_SUCCESS;
}

void print_bmp_header(const BmpHeader* bmp)
{
    fprintf(stdout, sssFormat, "BMP Header", "Data", "Hex");
    fputs(lineSeparator, stdout);

    // Output capped at two chars to prevent buffer overflow
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

int read_pixel_row(
        FILE* file, Image* image, const int rowNumber, const size_t byteOffset)
{
    const size_t numPixels = (size_t)(image->width);

    // Read row of pixels
    if (fread((image->pixels)[rowNumber], sizeof(Pixel), numPixels, file)
            != numPixels) {

        // Print message upon read error
        fprintf(stderr, errorReadingPixelsMessage, rowNumber);
        return -1;
    }

    if (byteOffset) { // If offset non-zero update file pointer
        fseek(file, (long)byteOffset, SEEK_CUR);
    }

    return EXIT_SUCCESS;
}

Image* load_bmp_2d(FILE* file, const BmpHeader* restrict header,
        const BmpInfoHeader* restrict bmp)
{
    // Initialise pixel array
    Image* image = create_image(bmp->bitmapWidth, bmp->bitmapHeight);

    if (image == NULL) {
        fputs(bmpLoadFailMessage, stderr);
        return NULL;
    }

    // Calculate offset required due to row padding (32-bit DWORD length)
    const size_t byteOffset
            = calc_row_byte_offset(bmp->bitsPerPixel, bmp->bitmapWidth);

    // Seek to start of pixel data
    fseek(file, header->offset, SEEK_SET);

    // For each row of pixels
    for (int height = 0; height < bmp->bitmapHeight; height++) {
        if (read_pixel_row(file, image, height, byteOffset) == -1) {
            free_image(&image);
            fputs(bmpLoadFailMessage, stderr);
            return NULL;
        }
    }

    const long endAddr = ftell(file); // Could check for error

    // Print offset of file pointer after iterating all pixels
    printf(eofAddrMessage, endAddr);

    if ((int64_t)endAddr != (int64_t)(header->bmpSize)) {
        fprintf(stderr, eofMismatchMessage, endAddr, header->bmpSize);
        // Could add error return value
    }

    return image;
}

void print_image_to_terminal(const Image* image)
{
    // Initialise
    char buffer[OUTPUT_BUFFER_CAPACITY];
    size_t bufferPosition = 0;

    // For each pixel (RGB)
    for (int height = 0; height < image->height; height++) {
        for (int width = 0; width < image->width; width++) {

            // If buffer does not have room for pixel, write buffer to
            // terminal and reset buffer position.
            if ((bufferPosition + MAX_ANSI_PIXEL_LEN)
                    >= OUTPUT_BUFFER_CAPACITY) {
                fwrite(buffer, 1, bufferPosition, stdout);
                bufferPosition = 0;
            }
            Pixel p = (image->pixels)[height][width];

            // Append pixel to buffer
            bufferPosition += (size_t)sprintf(&buffer[bufferPosition],
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

int read_pixel(uint8_t (*pixel)[RGB_PIXEL_BYTE_SIZE], FILE* file)
{
    // For each colour (RGB)
    for (int colour = 1; colour <= RGB_PIXEL_BYTE_SIZE; colour++) {

        // Reads intensity of colour and stores into pixel
        if (fread(&((*pixel)[RGB_PIXEL_BYTE_SIZE - colour]), sizeof(uint8_t), 1,
                    file)
                != 1) {
            fputs(bmpLoadFailMessage, stderr);
            return -1;
        }
    }

    return EXIT_SUCCESS;
}

size_t calc_row_byte_offset(
        const uint16_t bitsPerPixel, const int32_t bitmapWidth)
{
    // Calculate offset required due to row padding (32-bit DWORD len)
    const size_t byteOffset
            = (BMP_ROW_DWORD_LEN
                      - (((size_t)bitsPerPixel * (size_t)bitmapWidth)
                              % BMP_ROW_DWORD_LEN))
            % BMP_ROW_DWORD_LEN;
    return byteOffset / SIZE_BYTE;
}

Image* create_image(const int32_t width, const int32_t height)
{
    Image* img = malloc(sizeof(Image));

    // If malloc fails
    if (img == NULL) {
        return NULL;
    }

    img->width = width;
    img->height = height;

    size_t normHeight;
    if (height < 0) {
        normHeight = (size_t)(-height);
    } else {
        normHeight = (size_t)height;
    }

    // Allocate memory for array of row pointers
    img->pixels = (Pixel**)malloc(normHeight * sizeof(Pixel*));

    if (img->pixels == NULL) { // Malloc failed
        free(img);
        return NULL;
    }

    // Allocate memory for all pixel data
    img->pixelData = malloc((size_t)width * normHeight * sizeof(Pixel));

    if (img->pixelData == NULL) { // If malloc fails
        free(img->pixels);
        free(img);
        return NULL;
    }

    // Link the pixel data to each row
    for (size_t i = 0; i < normHeight; i++) {
        (img->pixels)[i] = &(img->pixelData)[(size_t)width * i];
    }

    return img;
}

void flip_image(Image* image)
{
    const int32_t last = image->height >> 1;
    for (int32_t y = 0; y < last; y++) {

        Pixel* tempRow = (image->pixels)[y];
        const int32_t bottomRow = image->height - y - 1;

        (image->pixels)[y] = (image->pixels)[bottomRow];
        (image->pixels)[bottomRow] = tempRow;
    }
}

void write_padding(FILE* file, const size_t gapSize)
{
    const uint8_t zero = 0;

    for (size_t i = 0; i < gapSize; i++) {
        fwrite(&zero, 1, 1, file);
    }
}

int write_bmp_with_header_provided(BMP* bmpImage, const char* filename)
{
    BmpHeader* bmpHeader = &(bmpImage->bmpHeader);
    BmpInfoHeader* infoHeader = &(bmpImage->infoHeader);
    Image* image = bmpImage->image;

    FILE* output = fopen(filename, writeMode);
    if (check_file_opened(output, filename) == -1) {
        return -1;
    }

    // Write BmpHeader
    fwrite(&bmpHeader->id, sizeof(bmpHeader->id), 1, output);
    fwrite(&bmpHeader->bmpSize, sizeof(bmpHeader->bmpSize), 1, output);
    fwrite(&bmpHeader->reserved1, sizeof(bmpHeader->reserved1), 1, output);
    fwrite(&bmpHeader->reserved2, sizeof(bmpHeader->reserved2), 1, output);
    fwrite(&bmpHeader->offset, sizeof(bmpHeader->offset), 1, output);

    // Write BmpInfoHeader
    fwrite(&infoHeader->headerSize, sizeof(infoHeader->headerSize), 1, output);
    fwrite(&infoHeader->bitmapWidth, sizeof(infoHeader->bitmapWidth), 1,
            output);
    fwrite(&infoHeader->bitmapHeight, sizeof(infoHeader->bitmapHeight), 1,
            output);
    fwrite(&infoHeader->colourPlanes, sizeof(infoHeader->colourPlanes), 1,
            output);
    fwrite(&infoHeader->bitsPerPixel, sizeof(infoHeader->bitsPerPixel), 1,
            output);
    fwrite(&infoHeader->compression, sizeof(infoHeader->compression), 1,
            output);
    fwrite(&infoHeader->imageSize, sizeof(infoHeader->imageSize), 1, output);
    fwrite(&infoHeader->horzResolution, sizeof(infoHeader->horzResolution), 1,
            output);
    fwrite(&infoHeader->vertResolution, sizeof(infoHeader->vertResolution), 1,
            output);
    fwrite(&infoHeader->coloursInPalette, sizeof(infoHeader->coloursInPalette),
            1, output);
    fwrite(&infoHeader->importantColours, sizeof(infoHeader->importantColours),
            1, output);

    const long currentPosition = ftell(output);

    if (!(currentPosition < 0) && (currentPosition < bmpHeader->offset)) {
        const size_t gapSize = (size_t)(bmpHeader->offset - currentPosition);
        write_padding(output, gapSize);
    }

    const size_t byteOffset = calc_row_byte_offset(
            infoHeader->bitsPerPixel, infoHeader->bitmapWidth);

    for (int row = 0; row < infoHeader->bitmapHeight; row++) {
        fwrite(image->pixels[row],
                (size_t)infoHeader->bitmapWidth * sizeof(Pixel), 1, output);

        if (byteOffset) {
            write_padding(output, byteOffset);
        }
    }

    safely_close_file(output);
    return EXIT_SUCCESS;
}

int check_file_opened(FILE* file, const char* const filePath)
{
    if (file == NULL) { // Check if file can be opened
        fprintf(stderr, fileOpeningErrorMessage, filePath);
        return -1;
    }

    return EXIT_SUCCESS;
}
