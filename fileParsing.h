// Exit codes
#define EXIT_OK 0
#define EXIT_INVALID_ARG 20
#define EXIT_FILE_CANNOT_BE_READ 9
#define EXIT_FILE_PARSING_ERROR 8

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
const char* const bitMap = "BM";

// Assorted constant chars
const char* const readMode = "r";
#define EOS '\0'

// Function prototypes
void parse_bit_map_header(FILE* file);
