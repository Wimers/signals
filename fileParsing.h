#ifndef FILEPARSING_H
#define FILEPARSING_H

// Included Libraries
#include <stdint.h>
#include <stdio.h>

// Exit codes
#define EXIT_FILE_INTEGRITY 7
#define EXIT_FILE_PARSING_ERROR 8
#define EXIT_OUT_OF_BOUNDS 10

// File constants
#define HEADER_FIELD_SIZE 2
#define BMP_FILE_SIZE 4
#define RESERVED1 2
#define RESERVED2 2
#define OFFSET_ADDR_SIZE 4
#define BITMAP_FILE_HEADER_SIZE 14
#define DIB_HEADER_SIZE 7
#define BI_RGB 0 // compression method
#define HALFTONING_ALGORITHM 0 // None

#define RGB_PIXEL_BYTE_SIZE 3
#define SIZE_BYTE 8
#define BMP_ROW_DWORD_LEN 32
#define VERT_TERMINAL_RESOLUTION 1
#define MAX_ANSI_PIXEL_LEN 32
#define OUTPUT_BUFFER_CAPACITY 8192

// Program constant strings
extern const char* const invalidColourPlanesMessage;
extern const char* const bitMap;
extern const char* const newlineStr;
extern const char* const eofAddrMessage;
extern const char* const gradient;
extern const char* const colouredBlockFormatter;

// Assorted constant chars
extern const char newlineChar;
#define EOS '\0'

typedef struct { // 14 bytes
    uint16_t id;
    uint32_t bmpSize;
    uint32_t offset;
} BmpHeader;

// DIB header (bitmap information header)
typedef struct { // 40 bytes
    uint32_t headerSize;
    int16_t bitmapWidth;
    int16_t bitmapHeight;
    uint16_t colourPlanes;
    uint16_t bitsPerPixel;
    uint32_t compression;
    uint32_t imageSize;
    uint32_t horzResolution;
    uint32_t vertResolution;
    uint32_t coloursInPalette;
    uint32_t importantColours;
} BmpInfoHeader;

typedef struct {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
} Pixel;

typedef struct {
    uint16_t width;
    uint16_t height;
    Pixel** pixels;
} Image;

/* parse_bmp_header()
 * ------------------
 * Parses the provided file, and stores the associated bmp metadata,
 * into the BMP header struct.
 *
 * It is assumed the input file is a .BMP file.
 *
 * bmp: Pointer to struct to store BMP Header metadata.
 * file: BMP file to parse.
 */
void parse_bmp_header(BmpHeader* bmp, FILE* file);

/* parse_bmp_info_header()
 * -----------------------
 * Parses the provided file, and stores the associated bmp metadata,
 * into the BMP info header struct.
 *
 * It is assumed the input file is .BMP file.
 *
 * bmp: Pointer to struct to store BMP Info Header metadata.
 * file: BMP file to parse.
 *
 * Errors: Exits with EXIT_FILE_INTEGRITY if the number of colour planes of file
 *         is not one.
 */
void parse_bmp_info_header(BmpInfoHeader* bmp, FILE* file);

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

/* brightness_gradient_mapping()
 * -----------------------------
 * Maps the input brightness to a char, and prints the char to stdout.
 * It is assumed the brightness is between 0 and 255 (inclusive).
 * Lower brightness values map to chars with a greater proportion of whitespace.
 */
void brightness_gradient_mapping(const int brightness);

void dump_headers(const BmpHeader* bmp, const BmpInfoHeader* infoHeader);
void read_headers(BmpHeader* restrict bmp, BmpInfoHeader* restrict infoHeader,
        FILE* file);
void read_pixel(uint8_t (*pixel)[RGB_PIXEL_BYTE_SIZE], FILE* file);
void get_pixel(const int x, const int y, const BmpHeader* restrict header,
        const BmpInfoHeader* restrict bmp, FILE* file);
uint32_t calc_row_byte_offset(const int bitsPerPixel, const int bitmapWidth);
void read_pixel_row(
        FILE* file, Image* image, int rowNumber, uint32_t byteOffset);
Image* create_image(uint32_t width, uint32_t height);
Image* load_bmp_2d(FILE* file, const BmpHeader* restrict header,
        const BmpInfoHeader* restrict bmp);
void free_image(Image* image);
void print_image_to_terminal(const Image* image);

#endif
