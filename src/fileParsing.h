#ifndef FILEPARSING_H
#define FILEPARSING_H

// Included Libraries
#include <stdint.h>
#include <stdio.h>
#include "pixels.h"

// Exit codes
#define EXIT_FILE_INTEGRITY 7
#define EXIT_FILE_PARSING_ERROR 8
#define EXIT_OUT_OF_BOUNDS 10
#define EXIT_HEADER_SAFETY 22

#define SIZE_BYTE 8

#define suXFormat "%-25s %-15u %X\n"
#define sdXFormat "%-25s %-15d %X\n"
#define sssFormat "%-25s %-15s %s\n"
#define ssdFormat "%-25s %-15s %d\n"
#define sudFormat "%-25s %-15u %d\n"

#define READ_HEADER_SAFE(dest, size, file, parameterName)                      \
    (void)sizeof(char[(sizeof(*(dest)) == (size)) ? 1 : -1]);                  \
    if (fread(dest, size, 1, file) != 1) {                                     \
        fprintf(stderr, "Error reading \"%s\".\n", parameterName);             \
        return -1;                                                             \
    }

typedef struct { // 14 bytes
    uint16_t id;
    uint32_t bmpSize;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset;
} BmpHeader;

// DIB header (bitmap information header)
typedef struct { // 40 bytes
    uint32_t headerSize;
    int32_t bitmapWidth;
    int32_t bitmapHeight;
    uint16_t colourPlanes;
    uint16_t bitsPerPixel;
    uint32_t compression;
    uint32_t imageSize;
    int32_t horzResolution;
    int32_t vertResolution;
    uint32_t coloursInPalette;
    uint32_t importantColours;
} BmpInfoHeader;

static inline Pixel* get_pixel_fast(
        const Image* restrict image, const size_t x, const size_t rowOffset)
{
    return &((image->pixelData)[x + rowOffset]);
}

static inline Pixel* get_pixel(Image* image, const size_t x, const size_t y)
{
    return &((image->pixelData)[y * image->width + x]);
}

typedef struct {
    FILE* file;
    BmpHeader bmpHeader;
    BmpInfoHeader infoHeader;
    Image* image;
} BMP;

/* initialise_bmp()
 * ----------------
 * bmpImage:
 */
void initialise_bmp(BMP* bmpImage);

/* free_image()
 * ------------
 * Frees all memory associated with the provided image struct.
 *
 * image: Pointer to the pointer of the image struct to free.
 */
void free_image(Image** image);

/* free_image_resources()
 * ----------------------
 * bmpImage:
 */
void free_image_resources(BMP* bmpImage);

int open_bmp(BMP* bmpImage, const char* const filePath);

/* read_headers()
 * --------------
 * Reads and parses both the File Header and Info Header of a BMP file.
 * Clears the destination structs and populates them by reading from the
 * provided file.
 *
 * bmpImage:
 */
int read_headers(BMP* bmpImage);

/* dump_headers()
 * --------------
 * Prints the contents of the BMP headers to the console.
 */
void dump_headers(const BMP* bmpImage);

/* parse_bmp_header()
 * ------------------
 * Parses the provided file, and stores the associated bmp metadata,
 * into the BMP header struct.
 *
 * It is assumed the input file is a .BMP file.
 */
int parse_bmp_header(BMP* bmpImage);

/* parse_bmp_info_header()
 * -----------------------
 * Parses the BMP file header from the provided file, and stores the associated
 * bmp metadata into the BMP info header struct.
 *
 * It is assumed the input file is .BMP file.
 *
 * Errors: Exits with EXIT_FILE_INTEGRITY if the number of colour planes of file
 *         is not one.
 */
int parse_bmp_info_header(BMP* bmpImage);

/* print_bmp_header()
 * ------------------
 * Prints the values of each parameter from the BMP Info header parsed.
 *
 * bmp: Struct containing all parsed BMP Header metadata.
 */
void print_bmp_header(const BmpHeader* bmp);

/* print_bmp_info_header()
 * -----------------------
 * Prints the values of each parameter from the BMP Info header parsed.
 *
 * bmp: Pointer to struct containing all parsed BMP Info Header metadata.
 */
void print_bmp_info_header(const BmpInfoHeader* bmp);

/* read_pixel_row()
 * ----------------
 * Reads a single row of pixels from the file into the Image struct.
 *
 * file: File stream to the open file.
 * image: The Image struct containing the pixel data.
 * rowNumber: The current row index (height) being read.
 * byteOffset: The number of padding bytes to skip after reading the row.
 *
 * Returns:
 */
int read_pixel_row(FILE* file, Image* image, const size_t rowNumber,
        const size_t byteOffset);

/* load_bmp()
 * -------------
 * Loads the entire pixel array from the BMP file.
 *
 * file: File stream to the open file.
 * header: Struct containing all parsed BMP Header metadata.
 * bmp: Pointer to struct containing all parsed BMP Info Header metadata.
 *
 * Returns: Pointer to the image struct containing the images pixel data.
 */
Image* load_bmp(FILE* file, const BmpHeader* restrict header,
        const BmpInfoHeader* restrict bmp);

/* print_image_to_terminal()
 * -------------------------
 * Renders the image to the terminal, using block characters and ANSI escape
 * codes to render to print each pixel in colour.
 *
 * image: The image struct containing the pixel data.
 */
void print_image_to_terminal(const Image* image);

/* calc_row_byte_offset()
 * ----------------------
 * Calculates the number of padding bytes required for BMP allignment.
 * BMP rows must be alligned to the 32-bit DWORD length.
 *
 * bitsPerPixel: Image colour depth.
 * bitmapWidth: Width of the image in pixel.
 *
 * Returns: The number of padding bytes per row.
 */
size_t calc_row_byte_offset(
        const uint16_t bitsPerPixel, const int32_t bitmapWidth);

/* create_image()
 * --------------
 * Allocates memory for an Image struct and its pixel data.
 *
 * width: Image width in pixels.
 * height: Image height in pixels.
 *
 * Returns: Pointer to the newly allocated Image structure.
 */
Image* create_image(const int32_t width, const int32_t height);

/* write_bmp_with_header_provided()
 * --------------------------------
 * bmp:
 * infoHeader:
 * image:
 * filename:
 *
 */
int write_bmp_with_header_provided(
        BMP* bmpImage, const char* filename, const char* messagePath);

/* check_file_opened()
 * -------------------
 * file:
 * filePath:
 */
int check_file_opened(FILE* file, const char* const filePath);

/* write_padding()
 * ---------------
 * file:
 * gapSize:
 */
void write_padding(FILE* file, const size_t gapSize);

int header_safety_checks(BMP* bmpImage);
int handle_bmp_loading(BMP* bmpImage);
void check_image_resolution(BmpInfoHeader* info);
void safely_close_file(FILE* file);
void write_pixel_data(
        FILE* output, BmpHeader* bmpHeader, BmpInfoHeader* info, Image* image);
[[nodiscard]] bool write_padding_message(FILE* dest, FILE* src, size_t gapSize);
void write_padding_zeros(FILE* file, size_t gapSize);
[[nodiscard]] int write_pixel_data_secret(FILE* output, BmpHeader* bmpHeader,
        BmpInfoHeader* info, Image* image, const char* messagePath);
#endif
