#include <stdlib.h>
#include <string.h>
#include "imageEditing.h"
#include "fileParsing.h"
#include "filters.h"

int flip_image(Image* image)
{
    if (image == NULL) {
        return -1;
    }

    const size_t rowSize = image->width * sizeof(Pixel);
    Pixel* rowBuffer = malloc(rowSize);

    if (rowBuffer == NULL) {
        perror("malloc failed while flipping..");
        return -1;
    }

    const size_t last = image->height >> 1;
    for (size_t y = 0; y < last; y++) {

        // Calculate offsets for starting index of current top/bottom rows
        size_t topOffset = image->width * y;
        size_t bottomOffset = (image->height - y - 1) * image->width;

        Pixel* topRow = &(image->pixelData)[topOffset];
        Pixel* bottomRow = &(image->pixelData)[bottomOffset];

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
    // Only need to iterate throgh half the width
    const size_t iterBound = image->width >> 1;

    // For each row in the image
    for (size_t y = 0; y < image->height; y++) {
        size_t rowOffset = image->width * y;
        size_t last = image->width - 1;

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
    // Involves casting size_t to int32_t, this is safe provided the input image
    // was generated using create_image, which converts the size read from the
    // file header to a size_t. So no overflow can occur
    Image* transpose
            = create_image((int32_t)(image->height), (int32_t)(image->width));

    if (transpose != NULL) {
        FX_TEMPLATE(image, { // Swap (x, y) -> (y, x)
            (transpose->pixelData)[(transpose->width * x) + y] = *pixel;
        });
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
