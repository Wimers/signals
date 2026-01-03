#ifndef IMAGE_H
#define IMAGE_H

#include "fileParsing.h"

/* flip_image()
 * ------------
 * Flips the input image upside down in-place.
 *
 * It swaps the top pixel rows with the corresponding bottom pixel rows using a
 * temporary row buffer. This modifies the existing image data directly.
 *
 * image: Pointer to the Image to be flipped.
 *
 * Returns: 0 on success, or -1 if the input image is NULL
 *          or if memory allocation for the row buffer fails.
 */
[[nodiscard]] int flip_image(Image* image);

/* reverse_image()
 * ---------------
 * Mirrors an image horizontally in-place.
 *
 * Iterates through every row and reverses the order of pixels.
 * This operation modifies the existing image structure in place.
 *
 * image: Pointer to the Image to be reversed.
 */
void reverse_image(Image* image);

/* transpose_image()
 * -----------------
 * Creates a new image that is a transposed copy of the input.
 *
 * Swaps the width and height dimensions. The pixel at (x, y) in the input
 * becomes the pixel at (y, x) in the output.
 *
 * image: Pointer to the source Image.
 *
 * Return: A pointer to the new transposed Image, or NULL if
 * 	   memory allocation fails.
 *
 * Notes
 * -----
 * It is assumed the input image was processed via the create_image prior to
 * calling. This is to ensure the width/height members of the image are safe to
 * cast to size_t.
 *
 * The caller is responsible for freeing the returned image.
 */
Image* transpose_image(const Image* restrict image);

/* rotate_image_clockwise()
 * ------------------------
 * Creates a new image rotated 90° clockwise.
 *
 * Transposes and then vertically flips the input image storing the result in
 * a newly allocated Image. It is the callers responsibility to free the
 * returned Image.
 *
 * image: Pointer to the source Image
 *
 * Returns: Pointer to the new rotated Image, or NULL upon failure.
 */
Image* rotate_image_clockwise(const Image* restrict image);

/* rotate_image_anticlockwise()
 * ----------------------------
 * Creates a new image rotated 90° anti-clockwise.
 *
 * Transposes and then horizontally reverses the input image storing the result
 * in a newly allocated image. It is the callers responsibility to free the
 * returned image.
 *
 * image: Pointer to the source image
 *
 * Returns: Pointer to the new rotated image, or NULL upon failure.
 */
Image* rotate_image_anticlockwise(const Image* restrict image);

#endif
