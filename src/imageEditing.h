#ifndef IMAGE_H
#define IMAGE_H

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
void flip_image(Image* image);

void reverse_image(Image* image);

Image* rotate_image(Image* image, const long nRotations);
Image* rotate_image_clockwise(Image* image);
Image* rotate_image_anticlockwise(Image* image);

#endif
