#ifndef FILEPARSING_H
#define FILEPARSING_H

// Included Libraries
#include <stdint.h>
#include <stdio.h>

// Exit codes
#define EXIT_FILE_INTEGRITY 7
#define EXIT_FILE_PARSING_ERROR 8
#define EXIT_OUT_OF_BOUNDS 10
#define EXIT_HEADER_SAFETY 22

// File constants
#define BMP_HEADER_SIZE 14
#define DIB_HEADER_SIZE 40
#define OFFSET_ADDR_SIZE 4

// BMP compression modes, BI_RGB (none) is the default compression method
#define BI_RGB 0
#define COMP_METH_VAL_MAX 13 // (BMP standard allows values range from 0 <-> 13)

#define RGB_PIXEL_BYTE_SIZE 3
#define SIZE_BYTE 8
#define BMP_ROW_DWORD_LEN 32
#define MAX_ANSI_PIXEL_LEN 32
#define OUTPUT_BUFFER_CAPACITY 8192

#define suXFormat "%-25s %-15u %X\n"
#define sdXFormat "%-25s %-15d %X\n"
#define sssFormat "%-25s %-15s %s\n"
#define ssdFormat "%-25s %-15.2s %d\n"
#define sudFormat "%-25s %-15u %d\n"

// Program constant strings
extern const char* const invalidColourPlanesMessage;
extern const char* const fileOpeningErrorMessage;
extern const char* const errorReadingHeaderMessage;
extern const char* const negativeWidthMessage;
extern const char* const errorReadingPixelsMessage;
extern const char* const resettingIntValueMessage;
extern const char* const bmpLoadFailMessage;
extern const char* const eofMismatchMessage;
extern const char* const bitMap;
extern const char* const eofAddrMessage;
extern const char* const colouredBlockFormatter;
extern const char* const newlineStr;

// Assorted constant chars
extern const char newlineChar;
#define EOS '\0'

#define READ_HEADER_SAFE(dest, size, file, parameterName)                      \
    (void)sizeof(char[(sizeof(*(dest)) == (size)) ? 1 : -1]);                  \
    if (fread(dest, size, 1, file) != 1) {                                     \
        fprintf(stderr, errorReadingHeaderMessage, parameterName);             \
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

typedef struct __attribute__((packed)) {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
} Pixel;

typedef struct {
    int32_t width;
    int32_t height;
    Pixel* pixelData;
    Pixel** pixels;
} Image;

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

/* load_bmp_2d()
 * -------------
 * Loads the entire 2D pixel array from the BMP file.
 *
 * file: File stream to the open file.
 * header: Struct containing all parsed BMP Header metadata.
 * bmp: Pointer to struct containing all parsed BMP Info Header metadata.
 *
 * Returns: Pointer to the image struct containing the images pixel data.
 */
Image* load_bmp_2d(FILE* file, const BmpHeader* restrict header,
        const BmpInfoHeader* restrict bmp);

/* print_image_to_terminal()
 * -------------------------
 * Renders the image to the terminal, using block characters and ANSI escape
 * codes to render to print each pixel in colour.
 *
 * image: The image struct containing the pixel data.
 */
void print_image_to_terminal(const Image* image);

/* read_pixel()
 * ------------
 * Reads a single pixels RGB data from the provided file, and stores it in the
 * pixel array.
 *
 * pixel: Pointer to a buffer capable of holding RGB_PIXEL_BYTE_SIZE bytes.
 * file: File stream to the open file.
 *
 * Returns:
 */
int read_pixel(uint8_t (*pixel)[RGB_PIXEL_BYTE_SIZE], FILE* file);

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

/* flip_image()
 * ------------
 * Flips the input image upside down by replacing top pixel rows moving down,
 * with their corresponding pixel row from the bottom moving up. This requires
 * duplicating all data from the input image, creating a new image with the
 * rows flipped, freeing the old image, and returning a pointer to the new
 * image.
 *
 * image: Pointer to the image struct to be flipped.
 */
void flip_image(Image* image);

/* write_bmp_with_header_provided()
 * --------------------------------
 * bmp:
 * infoHeader:
 * image:
 * filename:
 *
 */
int write_bmp_with_header_provided(BMP* bmpImage, const char* filename);

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

int is_str_in_const_str_array(const void* restrict arg,
        const char* const strArray[], const size_t nread);
int header_safety_checks(BMP* bmpImage);
int handle_bmp_loading(BMP* bmpImage);
void check_image_resolution(BmpInfoHeader* info);

#endif
