// Included Libraries
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Exit codes
#define EXIT_INVALID_ARG 20
#define EXIT_FILE_CANNOT_BE_READ 9
#define EXIT_FILE_PARSING_ERROR 8
#define EXIT_OK 0

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

// Functino prototypes
void parse_bit_map_header(FILE* file);

// Program constant strings
const char* const fileOpeningErrorMessage
        = "The provided file \"%s\" cannot be opened for reading\n";
const char* const invalidArgsMessage = "Invalid arguments supplied\n";
const char* const bitMap = "BM";

// Assorted constant chars
const char* const readMode = "r";
const char endOfString = '\0';

int main(int argc, char** argv)
{
    if (argc != 2) {
        fputs(invalidArgsMessage, stderr);
        exit(EXIT_INVALID_ARG);
    }

    const char* filePath = argv[1];
    if (filePath[0] == endOfString) {
        exit(EXIT_INVALID_ARG);
    }

    FILE* file = fopen(filePath, readMode);
    if (file == NULL) { // Check if file can be opened
        fprintf(stderr, fileOpeningErrorMessage, filePath);
        exit(EXIT_FILE_CANNOT_BE_READ);
    }

    (void)parse_bit_map_header(file);

    exit(EXIT_OK);
}

void parse_bit_map_header(FILE* file)
{
    int bytesRead = 0;
    uint16_t fileType = 0;

    bytesRead = fread(&fileType, 2, 1, file);
    if (bytesRead != 1) {
        exit(EXIT_FILE_PARSING_ERROR);
    } else {
        uint16_t convertedVal = *(uint16_t*)bitMap;
        if (convertedVal != fileType) {
            exit(EXIT_FILE_PARSING_ERROR);
        }
    }

    uint32_t fileSize = 0;
    bytesRead = fread(&fileSize, 4, 1, file);

    // Print number of bytes in the input file data sector
    fprintf(stdout, "%d\n", fileSize);

    fclose(file);
}
