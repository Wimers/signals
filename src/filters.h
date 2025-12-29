#ifndef FILTERS_H
#define FILTERS_H

#include "fileParsing.h"
#include <stdint.h>

// Constants to best map RGB values to grayscale
// Each multiplied by 1024 to avoid floats
constexpr int rMapGS = 306; // 0.299
constexpr int gMapGS = 601; // 0.587
constexpr int bMapGS = 117; // 0.114

// Used to optimise division by 3 (for positive integers less than 765)
constexpr int div3Const = 683;
constexpr int div3Shift = 11;

// Used for division by 1024 (more efficient than dividing by 1000)
constexpr int pixScaleMultGS = 10;

#define FX_TEMPLATE(image, function)                                           \
    for (size_t y = 0; y < image->height; y++) {                               \
        size_t rowOffset = y * image->width;                                   \
        Pixel* rowPtr = get_pixel_fast(image, 0, rowOffset);                   \
                                                                               \
        _Pragma("omp simd") for (size_t x = 0; x < image->width; x++)          \
        {                                                                      \
            Pixel* pixel = rowPtr + x;                                         \
            function;                                                          \
        }                                                                      \
    }

static inline uint8_t calc_pixel_average(Pixel* pixel)
{
    return (uint8_t)(((pixel->red + pixel->green + pixel->blue) * div3Const)
            >> div3Shift);
}

static inline uint8_t calc_pixel_grayscale(Pixel* pixel)
{
    // Calculate gray scaled value
    const uint32_t temp = (uint32_t)(rMapGS * pixel->red + gMapGS * pixel->green
            + bMapGS * pixel->blue);

    return (uint8_t)(temp >> pixScaleMultGS);
}

/* invert_colours()
 * ----------------
 * Inverts the colour of each pixel.
 *
 * image: Pointer to struct containing the pixel data.
 */
void invert_colours(Image* image);

/* filter_red()
 * ------------
 * Sets the red component of each pixel to zero.
 *
 * image: Pointer to struct containing the pixel data.
 */
void filter_red(Image* image);

/* filter_green()
 * --------------
 * Sets the green component of each pixel to zero.
 *
 * image: Pointer to struct containing the pixel data.
 */
void filter_green(Image* image);

/* filter_blue()
 * -------------
 * Sets the green component of each pixel to zero.
 *
 * image: Pointer to struct containing the pixel data.
 */
void filter_blue(Image* image);

void filter_red_green(Image* image);
void filter_red_blue(Image* image);
void filter_green_blue(Image* image);
void filter_all(Image* image);

/* gray_filter()
 * -------------
 * Applies a gray scale filter to the image. Modifies each pixel based on
 * weighted values for each pixel component based on how sensitive the human eye
 * is to different wavelengths of light.
 *
 * image: Pointer to struct containing the pixel data.
 */
void gray_filter(Image* image);

/* average_pixels()
 * ----------------
 * Applies a pixel averaging filter to an image, which sets each pixel to the
 * average intensity of the pixels components.
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
 * maxBrightness: Maximum brightness threshold.
 */
void brightness_cut_filter(Image* image, const uint8_t cutoff);

/* combine_images()
 * ----------------
 * Combines two images together, updating the primary image with the pixels
 * averaged from primary and secondary images.
 *
 * primary: Destination image to store combination.
 * secondary: Image combined with primary.
 *
 * Returns: EXIT_SUCCESS on success.
 *
 * Errors: Returns EXIT_OUT_OF_BOUNDS, if the images dimensions do not match.
 */
[[nodiscard]] int combine_images(
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
 * Errors: Returns -1 upon malloc failure, and if offset is out of bounds.
 */
[[nodiscard]] int glitch_effect(Image* image, const size_t glitchOffset);

/* verify_offset_bounds()
 * ----------------------
 * Checks if the offset is less than the images width. Prints error messages to
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
 * Applies a rough contrasting filter to an image.
 *
 * image: Pointer to struct containing the pixel data.
 * contrastFactor: Level of contrasting.
 * min: Lower bound of colour intensity to be uneffected.
 * max: Upper bound of colour intensity to be uneffected.
 */
void contrast_effect(Image* image, const uint8_t contrastFactor,
        const uint8_t min, const uint8_t max);

/* dim_effect()
 * ------------
 * Reduces the indensity of every pixel by a constant dimming factor.
 * Pixels are reduced to a minimum of zero to prevent integer underflow.
 *
 * image: Pointer to struct containing the pixel data.
 * dimmingFactor: Factor each pixel component is reduced by.
 */
void dim_effect(Image* image, const uint8_t redDim, const uint8_t greenDim,
        const uint8_t blueDim);

/* contrast_effect_val()
 * ---------------------
 * Calculates the resulting pixel intensity provided contrast effect parameters.
 *
 * val: Current intensity of a pixel component.
 * contrastFactor: Level of contrasting.
 * min: Lower bound of colour intensity to be uneffected.
 * max: Upper bound of colour intensity to be uneffected.
 *
 * Returns: Colour intensity for a pixel component.
 */
uint8_t contrast_effect_val(uint8_t val, const uint8_t contrastFactor,
        const uint8_t min, const uint8_t max);

/* swap_red_blue()
 * ---------------
 * Swaps the red and blue components of each pixel for a provided image.
 *
 * image: Pointer to struct containing the pixel data.
 */
void swap_red_blue(Image* image);

[[nodiscard]] int melt(BMP* bmp, const int32_t startPoint);

void colour_scaler(
        Image* image, const double red, const double green, const double blue);

[[nodiscard]] int merge_images(
        Image* restrict primary, const Image* restrict secondary);
Image* image_blur(Image* image, uint32_t radius);

#endif
