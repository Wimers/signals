#ifndef FILTERS_H
#define FILTERS_H

#include "fileParsing.h"
#include <stdint.h>

/* FX_TEMPLATE
 * -----------
 * Macro to iterate over every pixel in an image with SIMD optimisation.
 */
#define FX_TEMPLATE(image, function)                                           \
    for (size_t y = 0; y < image->height; y++) {                               \
        const size_t rowOffset = y * image->width;                             \
        Pixel* rowPtr = get_pixel_fast(image, 0, rowOffset);                   \
                                                                               \
        _Pragma("omp simd") for (size_t x = 0; x < image->width; x++)          \
        {                                                                      \
            Pixel* pixel = rowPtr + x;                                         \
            function;                                                          \
        }                                                                      \
    }

/* invert_colours()
 * ----------------
 * Inverts the colour of each pixel of an Image (creates a negative).
 *
 * image: Pointer to struct containing the pixel data.
 */
void invert_colours(Image* image);

/* filter_red()
 * ------------
 * Sets the red channel of each pixel in an Image to zero.
 *
 * image: Pointer to struct containing the pixel data.
 */
void filter_red(Image* image);

/* filter_green()
 * --------------
 * Sets the green channel of each pixel in an Image to zero.
 *
 * image: Pointer to struct containing the pixel data.
 */
void filter_green(Image* image);

/* filter_blue()
 * -------------
 * Sets the blue channel of each pixel in an Image to zero.
 *
 * image: Pointer to struct containing the pixel data.
 */
void filter_blue(Image* image);

/* filter_red_green()
 * ------------------
 * Sets the red and green channels of each pixel in an Image to zero.
 *
 * image: Pointer to struct containing the pixel data.
 */
void filter_red_green(Image* image);

/* filter_red_blue()
 * ------------------
 * Sets the red and blue channels of each pixel in an Image to zero.
 *
 * image: Pointer to struct containing the pixel data.
 */
void filter_red_blue(Image* image);

/* filter_green_blue()
 * ------------------
 * Sets the green and blue channels of each pixel in an Image to zero.
 *
 * image: Pointer to struct containing the pixel data.
 */
void filter_green_blue(Image* image);

/* filter_all()
 * ------------------
 * Sets each channel of each pixel in an Image to zero.
 *
 * image: Pointer to struct containing the pixel data.
 */
void filter_all(Image* image);

/* gray_filter()
 * -------------
 * Applies a gray scale filter to the Image. Modifies each pixel using the Luma
 * RGB coefficients to account for the sensitivity of the human eye to different
 * wavelengths of light.
 *
 * image: Pointer to struct containing the pixel data.
 */
void gray_filter(Image* image);

/* average_pixels()
 * ----------------
 * Applies a pixel averaging filter to an Image, which sets each pixel to the
 * average intensity of each of the pixels components.
 *
 * image: Pointer to struct containing the pixel data.
 */
void average_pixels(Image* image);

/* brightness_cut_filter()
 * -----------------------
 * Applies a brightness cut filter, which sets the component of a pixel to zero,
 * if its intensity is greater than the brightness cutoff.
 *
 * image: Pointer to struct containing the pixel data.
 * cutoff: Maximum brightness threshold.
 */
void brightness_cut_filter(Image* image, const uint8_t cutoff);

/* combine_images()
 * ----------------
 * Combines two images together, overwriting the primary image to store the
 * result. Each pixel is the average of the primary and secondary Images pixels.
 *
 * primary: Destination Image (and first source).
 * secondary: Source Image to add to the primary.
 *
 * Returns: EXIT_SUCCESS on success, EXIT_OUT_OF_BOUNDS if dimensions mismatch.
 */
[[nodiscard]] int combine_images(
        Image* restrict primary, const Image* restrict secondary);

/* merge_images()
 * --------------
 * Merges two images by adding their pixel values together.
 * The result is clamped above by UINT8_MAX to prevent overflow.
 *
 * primary: Destination image (and first source).
 * secondary: Source image to add to the primary.
 *
 * Returns: EXIT_SUCCESS on success, EXIT_OUT_OF_BOUNDS if dimensions mismatch.
 */
[[nodiscard]] int merge_images(
        Image* restrict primary, const Image* restrict secondary);

/* glitch_effect()
 * ---------------
 * Applies a "glitch" effect to an image, offsetting both the blue component and
 * red component of each pixel left/right respectively by a given pixel offset.
 *
 * Offset values must be smaller than the image width.
 *
 * image: Pointer to struct containing the pixel data.
 * glitchOffset: Pixel offset of glitch effect.
 *
 * Returns: EXIT_SUCCESS on successful image filtering.
 *
 * Errors: Returns -1 upon malloc failure, or if offset is out of bounds.
 */
[[nodiscard]] int glitch_effect(Image* image, const size_t glitchOffset);

/* verify_offset_bounds()
 * ----------------------
 * Checks if the offset is less than the Images width. Prints error messages to
 * stderr if condition not met.
 *
 * image: Pointer to struct containing the pixel data.
 * offset: Offset of glitch.
 *
 * Returns: EXIT_SUCCESS if within bounds, else returns -1.
 */
[[nodiscard]] int verify_offset_bounds(Image* image, const size_t offset);

/* contrast_effect()
 * -----------------
 * Applies a contrasting filter to an Image
 *
 * image: Pointer to struct containing the pixel data.
 * contrastFactor: Level of contrasting
 */
void contrast_effect(Image* image, const float contrastFactor);

/* dim_effect()
 * ------------
 * Reduces the intensity of each Pixel of an Image by a constant channel
 * specific dimming factor. Pixels are reduced to a minimum of zero to prevent
 * integer underflow.
 *
 * image: Pointer to struct containing the pixel data.
 * redDim: Factor red component is reduced by.
 * greenDim: Factor green component is reduced by.
 * blueDim: Factor blue component is reduced by.
 */
[[deprecated]] void dim_effect(Image* image, const uint8_t redDim,
        const uint8_t greenDim, const uint8_t blueDim);

/* swap_red_blue()
 * ---------------
 * Swaps the red and blue components of each pixel in an Image.
 *
 * image: Pointer to struct containing the pixel data.
 */
void swap_red_blue(Image* image);

/* melt()
 * ------
 * Applies a "pixel sorting" effect to the image, creating a melting appearance
 * (from top to bottom). Pixel columns are sorted by intensity (qsort) in
 * ascending order, and Images are rotated before and after sorting to minimise
 * cache misses.
 *
 * Rotation directions can be swapped using a negative `start` value. This has
 * the effect of "melting" the image from the bottom up.
 *
 * bmp: Pointer to the BMP structure containing the image.
 * start: Determines the sorting threshold and orientation.
 *        Negative values trigger an inverse rotation before sorting.
 *
 * Returns: EXIT_SUCCESS on success, or -1 on failure.
 */
[[nodiscard]] int melt(BMP* bmp, const int32_t start);

/* colour_scaler_strict()
 * ----------------------
 * Scales the RGB components of each Pixel in an Image by the provided floating
 * point channel multipliers. The result is clamped above by UINT8_MAX to
 * prevent overflow.
 *
 * image: Pointer to struct containing the pixel data.
 * red: Red component multiplier.
 * green: Green component multiplier.
 * blue: Blue component multiplier.
 */
void colour_scaler_strict(
        Image* image, const float red, const float green, const float blue);

/* colour_scaler()
 * ---------------
 * Scales the RGB components of each Pixel in an Image by the provided floating
 * point channel multipliers.
 *
 * image: Pointer to struct containing the pixel data.
 * red: Red component multiplier.
 * green: Green component multiplier.
 * blue: Blue component multiplier.
 *
 * Note
 * ----
 * uint8_t overflow from the float multiplication is not prevented, for strict
 * scaling (clamped above by UINT8_MAX) see colour_scaler_strict().
 */
void colour_scaler(
        Image* image, const float red, const float green, const float blue);

/* even_faster_image_blur()
 * ------------------------
 * Applies a separable box blur to the image. It blurs rows, transposes the
 * image, blurs the new rows (original columns), and transposes back.
 *
 * Algorithm complexity is O(1) (relative to radius). This blurring algorithm is
 * better suited for blurring with a large radii, as the overhead from
 * transposing the image twice is significant and counter-productive for small
 * radii.
 *
 * image: Pointer to struct containing the pixel data.
 * radius: The radius of the blur.
 *
 * Returns: A pointer to the new blurred Image, or NULL on failure.
 */
Image* even_faster_image_blur(const Image* restrict image, const size_t radius);

/* faster_image_blur()
 * -------------------
 * Applies a separable box blur using a hybrid approach.
 *
 * Vertical Pass (O(R))
 * --------------------
 * Calculates the average of the vertical column for each pixel by iterating
 * over the blur radius. Uses a pre-calculated lookup table of row offsets to
 * minimize multiplication overhead.
 *
 * Horizontal Pass (O(1))
 * ----------------------
 * Applies a sliding window blur to the vertically averaged row.
 *
 * This function is deprecated because the vertical pass scales linearly with
 * the radius, making it significantly slower than even_faster_image_blur() for
 * large blur radii.
 *
 * image: Pointer to struct containing the pixel data.
 * radius: The radius of the blur.
 *
 * Returns: A pointer to the new blurred Image, or NULL on failure.
 */
[[deprecated]] Image* faster_image_blur(
        const Image* restrict image, const size_t radius);

/* edge_detection()
 * ----------------
 * The algorithm is still in development... check back later :)
 *
 * image: Pointer to struct containing the pixel data.
 * threshold: Minimum difference between pixels to register as an edge.
 */
void edge_detection(Image* image, const int threshold);

void apply_hue(Image* image, const int red, const int green, const int blue);

#endif
