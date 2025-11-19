// Included Libraries
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fileParsing.h"

int main(int argc, char** argv)
{
    if (argc != 2) {
        fputs(invalidArgsMessage, stderr);
        exit(EXIT_INVALID_ARG);
    }

    const char* filePath = argv[FILE_PATH_IX];
    if (filePath[0] == EOS) {
        exit(EXIT_INVALID_ARG);
    }

    FILE* file = fopen(filePath, readMode);
    if (file == NULL) { // Check if file can be opened
        fprintf(stderr, fileOpeningErrorMessage, filePath);
        exit(EXIT_FILE_CANNOT_BE_READ);
    }

    BmpHeader bmp;
    memset(&bmp, 0, sizeof(bmp));
    (void)parse_bit_map_header(&bmp, file);

    exit(EXIT_OK);
}

void parse_bit_map_header(BmpHeader* bmp, FILE* file)
{
    int bytesRead = 0;

    bytesRead = fread(&(bmp->id), 2, 1, file);
    if (bytesRead != 1) {
        exit(EXIT_FILE_PARSING_ERROR);
    } else {
        uint16_t convertedVal = *(uint16_t*)bitMap;
        if (convertedVal != bmp->id) {
            exit(EXIT_FILE_PARSING_ERROR);
        }
    }

    bytesRead = fread(&(bmp->bmpSize), 4, 1, file);

    // Print number of bytes in the input file data sector
    fprintf(stdout, "%d\n", bmp->bmpSize);

    fclose(file);
}
