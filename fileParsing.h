// Included Libraries
#include <stdint.h>

// Exit codes
#define EXIT_OK 0
#define EXIT_INVALID_ARG 20
#define EXIT_FILE_CANNOT_BE_READ 9
#define EXIT_FILE_PARSING_ERROR 8
#define EXIT_FILE_INTEGRITY 7

// cmd line argument indexes
#define FILE_PATH_IX 1

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

// static const char test[6][3] = {"BM", "BA", "CI", "CP", "IC", "PT"};

// ID, size, unused, unused, offset
// static const int BITMAPHEADER[5] = {2, 4, 2, 2, 4};

// DIB HEADER
// number of bytes in DIB header from this point, width of bitmat in pixles,
// height of bitmap in pixles (positive for bottom to top pixle order), Number
// of colour planes used, number of bits per pixle, BI_RGB, size of raw bitmap
// data (including padding), Print resolution of the image, number of colours in
// the palette, 0 means all colours are important.

// static const int BITMAPINFOHEADER[11] = {4, 4, 4, 2, 2, 4, 4, 4, 4, 4, 4};

// Program constant strings
const char* const fileOpeningErrorMessage
        = "The provided file \"%s\" cannot be opened for reading\n";
const char* const invalidArgsMessage = "Invalid arguments supplied\n";
const char* const invalidColourPlanesMessage
        = "The number of colour planes must be 1, got \"%d\"\n";
const char* const bitMap = "BM";
const char* const lineSeparator
        = "--------------------------------------------------\n";
const char* const suXFormat = "%-25s %-15u %X\n";
const char* const sdXFormat = "%-25s %-15d %X\n";
const char* const sssFormat = "%-25s %-15s %s\n";
const char* const ssdFormat = "%-25s %-15s %d\n";
const char* const sudFormat = "%-25s %-15u %d\n";
const char* const newlineStr = "\n";

// Assorted constant chars
const char* const readMode = "r";
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
void print_bmp_header(BmpHeader bmp);

/* print_bmp_info_header()
 * -----------------------
 * Prints the values of each parameter from the BMP Info header parsed.
 *
 * bmp: Struct containing all parsed BMP Info Header metadata.
 */
void print_bmp_info_header(BmpInfoHeader bmp);
