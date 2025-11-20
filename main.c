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
    parse_bit_map_header(&bmp, file);
    fclose(file);

    print_bmp_header(bmp);

    exit(EXIT_OK);
}

void parse_bit_map_header(BmpHeader* bmp, FILE* file)
{
    // Store the value in the ID field
    fread(&(bmp->id), 2, 1, file);

    // Store the size of the BMP file
    fread(&(bmp->bmpSize), 4, 1, file);

    // Jump to pixle array offset, and store value
    fseek(file, 0x0A, SEEK_SET);
    fread(&(bmp->offset), 4, 1, file);
}

void print_bmp_header(BmpHeader bmp)
{
    fprintf(stdout, "%-15s %-15s %s\n", "Field Name", "Data", "Hex");
    fprintf(stdout, "---------------------------------------\n");
    fprintf(stdout, "%-15s %-15s %d\n", "ID", (char*)&(bmp.id), bmp.id);
    fprintf(stdout, "%-15s %-15u %X\n", "Size", bmp.bmpSize, bmp.bmpSize);
    fprintf(stdout, "%-15s %-15u %X\n", "Offset", bmp.offset, bmp.offset);
}
