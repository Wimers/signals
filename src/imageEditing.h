#ifndef IMAGE_H
#define IMAGE_H

#include "fileParsing.h"

/* flip_image()
 * ------------
 * Flips the input image upside down by replacing top pixel rows moving down,
 * with their corresponding pixel row from the bottom moving up. This requires
 * duplicating all data from the input image, creating a new image with the
 * rows flipped, freeing the old image, and returning a pointer to the new
 * image.
 *
 * image: Pointer to the image struct to be flipped.
 */
[[nodiscard]] int flip_image(Image* image);

void reverse_image(Image* image);
Image* transpose_image(const Image* restrict image);
Image* rotate_image(Image* image, const long nRotations);
Image* rotate_image_clockwise(const Image* restrict image);
Image* rotate_image_anticlockwise(const Image* restrict image);

#endif
