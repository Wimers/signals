#include <stdlib.h>
#include <string.h>
#include "fileParsing.h"
#include "filters.h"
#include "imageEditing.h"

[[nodiscard]] int flip_image(Image* image)
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

        size_t topOffset = image->width * y;
        size_t bottomOffset = (image->height - y - 1) * image->width;

        Pixel* topRow = &(image->pixelData)[topOffset];
        Pixel* bottomRow = &(image->pixelData)[bottomOffset];

        memcpy(rowBuffer, topRow, rowSize);
        memcpy(topRow, bottomRow, rowSize);
        memcpy(bottomRow, rowBuffer, rowSize);
    }

    free(rowBuffer);

    return EXIT_SUCCESS;
}

void reverse_image(Image* image)
{
    const size_t iterBound = image->width >> 1;

    for (size_t y = 0; y < image->height; y++) {
        size_t rowOffset = image->width * y;

        // For each pixel in row
        for (size_t x = 0; x < iterBound; x++) {
            size_t last = image->width - x - 1;

            Pixel* pixel1 = get_pixel_fast(image, x, rowOffset);
            Pixel* pixel2 = get_pixel_fast(image, last, rowOffset);

            Pixel temp = *pixel1;
            *pixel1 = *pixel2;
            *pixel2 = temp;
        }
    }
}

static inline Image* rotation_helper(const Image* restrict image)
{
    Image* rotated = create_image((int32_t)(image->height),
            (int32_t)(image->width)); // FIX type casts

    if (rotated != NULL) {

        FX_TEMPLATE(image, {
            size_t innerRowOffset = rotated->width * x;
            (rotated->pixelData)[innerRowOffset + y] = *pixel;
        });
    }

    return rotated;
}

Image* rotate_image_clockwise(const Image* restrict image)
{
    Image* rotated = rotation_helper(image);

    if (rotated == NULL) {
        return NULL;
    }

    if (flip_image(rotated) == -1) {
        free_image(&rotated);
        return NULL;
    }

    return rotated;
}

Image* rotate_image_anticlockwise(const Image* restrict image)
{
    Image* rotated = rotation_helper(image);

    if (rotated == NULL) {
        return NULL;
    }

    reverse_image(rotated);
    return rotated;
}
