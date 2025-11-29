// Included Libraries
#include <stdint.h>
#include <stdio.h>

// Exit codes
#define EXIT_OK 0
#define EXIT_INVALID_ARG 20
#define EXIT_FILE_CANNOT_BE_READ 9
#define EXIT_FILE_PARSING_ERROR 8
#define EXIT_FILE_INTEGRITY 7
#define EXIT_NO_COMMAND 6

// cmd line argument indexes
#define FILE_PATH_IX 2

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

// static const char test[6][3] = {"BM", "BA", "CI", "CP", "IC", "PT"};

// Program constant strings
const char* const usageMessage = "Options: ./bmp [-h] [-d] [-i] <file>\n";
const char* const fileTypeMessage = "Input file must be \".bmp\"\n";
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
const char* const eofAddrMessage = "End of File Addr: %lu\n";
const char* const gradient = " .:-=+#%@";
const char* const fileType = ".bmp";
const char* const colouredBlockFormatter = "\033[38;2;%d;%d;%dm██\033[0m";

// Assorted constant chars
const char* const readMode = "r";
#define EOS '\0'

const char* const optstring = "hdi"; // Defined program flags

typedef enum {
    INVALID = -1,
    HELP = 'h',
    HEADER_DUMP = 'd',
    DISPLAY_IMAGE = 'i',
} Flag;

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
void print_bmp_header(const BmpHeader* bmp);

/* print_bmp_info_header()
 * -----------------------
 * Prints the values of each parameter from the BMP Info header parsed.
 *
 * bmp: Pointer to struct containing all parsed BMP Info Header metadata.
 */
void print_bmp_info_header(const BmpInfoHeader* bmp);

/* display_image() // FIX
 * ---------------
 * Reads pixel data from the input .bmp file, and prints each row of pixels to
 * the terminal, converting RGB values to ASCII.
 *
 * header:
 * bmp:
 * file:
 */
void display_image(const BmpHeader* restrict header,
        const BmpInfoHeader* restrict bmp, FILE* file);

/* brightness_gradient_mapping()
 * -----------------------------
 * Maps the input brightness to a char, and prints the char to stdout.
 * It is assumed the brightness is between 0 and 255 (inclusive).
 * Lower brightness values map to chars with a greater proportion of whitespace.
 */
void brightness_gradient_mapping(const int brightness);

void check_file_opened(FILE* file, const char* const filePath);
void dump_headers(const BmpHeader* bmp, const BmpInfoHeader* infoHeader);
void early_argument_checks(const int argc, char** argv);
void read_headers(BmpHeader* restrict bmp, BmpInfoHeader* restrict infoHeader,
        FILE* file);
void check_for_empty_args(const int argc, char** argv);
void check_argument_validity(const int argc, char** argv);
Flag command_mapping(const char* command);
int ends_with(const char* const target, const char* arg);
void read_pixel(uint8_t (*pixel)[RGB_PIXEL_BYTE_SIZE], FILE* file);
void get_pixel(const int x, const int y, const BmpHeader* restrict header,
        const BmpInfoHeader* restrict bmp, FILE* file);
uint32_t calc_row_byte_offset(const int bitsPerPixel, const int bitmapWidth);
