#include "fileParsing.h"

void flip_image(Image* image)
{
    const size_t last = image->height >> 1;
    for (size_t y = 0; y < last; y++) {

        Pixel* tempRow = (image->pixels)[y];
        const size_t bottomRow = image->height - y - 1;

        (image->pixels)[y] = (image->pixels)[bottomRow];
        (image->pixels)[bottomRow] = tempRow;
    }
}

void reverse_image(Image* image)
{
    for (size_t y = 0; y < image->height; y++) {
        Pixel* row = (image->pixels)[y];

        // For each pixel in row
        for (size_t x = 0; x < (image->width >> 1); x++) {

            const size_t last = image->width - x - 1;
            Pixel temp = row[x];
            row[x] = row[last];
            row[last] = temp;
        }
    }
}

Image* rotation_helper(Image* image)
{
    Image* rotated = create_image((int32_t)(image->height),
            (int32_t)(image->width)); // FIX type casts

    if (rotated != NULL) {

        // For each row of pixels
        for (size_t y = 0; y < image->height; y++) {
            Pixel* row = (image->pixels)[y];

            for (size_t x = 0; x < image->width; x++) {
                (rotated->pixels)[x][y] = row[x];
            }
        }
    }

    return rotated;
}

Image* rotate_image_clockwise(Image* image)
{
    Image* rotated = rotation_helper(image);

    if (rotated == NULL) {
        return NULL;
    }

    flip_image(rotated);
    return rotated;
}

Image* rotate_image_anticlockwise(Image* image)
{
    Image* rotated = rotation_helper(image);

    if (rotated == NULL) {
        return NULL;
    }

    reverse_image(rotated);
    return rotated;
}
