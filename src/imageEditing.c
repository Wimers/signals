#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "imageEditing.h"
#include "filters.h"

int flip_image(Image* image)
{
    if (image == NULL) {
        return -1;
    }

    const size_t width = image->width;
    const size_t height = image->height;

    const size_t rowSize = width * sizeof(Pixel);
    Pixel* restrict rowBuffer = malloc(rowSize);

    if (rowBuffer == NULL) {
        perror("malloc failed while flipping..");
        return -1;
    }

    const size_t last = height >> 1;

    Pixel* restrict pixelData = image->pixelData;

    for (size_t y = 0; y < last; y++) {

        // Calculate offsets for starting index of current top/bottom rows
        Pixel* topRow = pixelData + (width * y);
        Pixel* bottomRow = pixelData + ((height - y - 1) * width);

        // Swap top and bottom rows, via a buffer
        memcpy(rowBuffer, topRow, rowSize);
        memcpy(topRow, bottomRow, rowSize);
        memcpy(bottomRow, rowBuffer, rowSize);
    }

    free(rowBuffer);
    return EXIT_SUCCESS;
}

void reverse_image(Image* image)
{
    const size_t width = image->width;
    const size_t height = image->height;

    // Only need to iterate throgh half the width
    const size_t iterBound = width >> 1;

    // For each row in the image
    for (size_t y = 0; y < height; y++) {
        size_t rowOffset = width * y;
        size_t last = width - 1;

        // For each pixel in row, get current pair of start/end pixels (working
        // inwards towards centre) and swap the pixels in place.
        for (size_t x = 0; x < iterBound; x++) {

            Pixel* pixel1 = get_pixel_fast(image, x, rowOffset);
            Pixel* pixel2 = get_pixel_fast(image, last, rowOffset);

            Pixel temp = *pixel1;
            *pixel1 = *pixel2;
            *pixel2 = temp;
            --last;
        }
    }
}

Image* transpose_image(const Image* restrict image)
{
    const size_t xHeight = image->height;
    const size_t xWidth = image->width;

    // Involves casting size_t to int32_t, this is safe provided the input image
    // was generated using create_image, which converts the size read from the
    // file header to a size_t. So no overflow can occur
    Image* transpose = create_image((int32_t)xHeight, (int32_t)xWidth);

    constexpr size_t blockSize = 16;
    Pixel buffer[blockSize * blockSize];
    memset(buffer, 0, sizeof(buffer));

    for (size_t y = 0; y < xHeight; y += blockSize) {
        for (size_t x = 0; x < xWidth; x += blockSize) {

            const size_t xDiff = xWidth - x;
            const size_t currentBlockW
                    = (blockSize < xDiff) ? (blockSize) : (xDiff);

            const size_t yDiff = xHeight - y;
            const size_t currentBlockH
                    = (blockSize < yDiff) ? (blockSize) : (yDiff);

            for (size_t row = 0; row < currentBlockH; row++) {
                const size_t sourceRowOffset = (y + row) * xWidth;
                const Pixel* restrict rowPtr
                        = get_pixel_fast(image, x, sourceRowOffset);

                for (size_t col = 0; col < currentBlockW; col++) {
                    buffer[(col * blockSize) + row] = rowPtr[col];
                }
            }

            for (size_t row = 0; row < currentBlockW; row++) {
                const size_t destRowOffset = (x + row) * xHeight;
                Pixel* restrict destPtr
                        = get_pixel_fast(transpose, y, destRowOffset);

                for (size_t col = 0; col < currentBlockH; col++) {
                    destPtr[col] = buffer[(row * blockSize) + col];
                }
            }
        }
    }
    return transpose;
}

Image* rotate_image_clockwise(const Image* restrict image)
{
    Image* output = transpose_image(image);

    if (output == NULL) {
        return NULL;
    }

    if (flip_image(output) == -1) {
        free_image(&output);
        return NULL;
    }

    return output;
}

Image* rotate_image_anticlockwise(const Image* restrict image)
{
    Image* output = transpose_image(image);

    if (output == NULL) {
        return NULL;
    }

    reverse_image(output);
    return output;
}
