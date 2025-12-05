#ifndef FILTERS_H
#define FILTERS_H

#include "fileParsing.h"
#include <stdint.h>

// Constants to best map RGB values to grayscale
// Each multiplied by 1024 to avoid floats
#define GS_RED_MAP 306 // 0.299
#define GS_GREEN_MAP 601 // 0.587
#define GS_BLUE_MAP 117 // 0.114

// Used to optimise division by 3 (for positive integers less than 765)
#define DIV_3_CONST 683
#define DIV_3_SHIFT 11

// Used for division by 1024 (more efficient than dividing by 1000)
#define GS_PIXEL_SCALING_FACTOR 10

extern const char* const fileDimensionMismatchMessage;
extern const char* const imageBoundsMessage;

/* invert_colour()
 * ---------------
 * image:
 */
void invert_colours(Image* image);

/* filter_red()
 * ------------
 * image:
 */
void filter_red(Image* image);

/* filter_green()
 * --------------
 * image:
 */
void filter_green(Image* image);

/* filter_blue()
 * -------------
 * image:
 */
void filter_blue(Image* image);

/* gray_filter()
 * -------------
 * image:
 */
void gray_filter(Image* image);

/* average_pixels()
 * ----------------
 * image:
 */
void average_pixels(Image* image);

/* brightness_cap_filter()
 * -----------------------
 * image:
 * maxBrightness:
 */
void brightness_cap_filter(Image* image, const uint8_t maxBrightness);

/* combine_images()
 * ----------------
 * primary:
 * secondary:
 *
 * Returns:
 */
int combine_images(Image* restrict primary, const Image* restrict secondary);

/* glitch_effect()
 * ---------------
 * image:
 * glitchOffset:
 *
 * Returns:
 */
int glitch_effect(Image* image, const int32_t glitchOffset);

/* verify_offset_bounds()
 * ----------------------
 * image:
 * offset:
 *
 * Returns:
 */
int verify_offset_bounds(Image* image, const int32_t offset);

/* contrast_effect()
 * -----------------
 * image:
 * contrastFactor:
 * min:
 * max:
 */
void contrast_effect(Image* image, const uint8_t contrastFactor,
        const uint8_t min, const uint8_t max);

/* dim_effect()
 * ------------
 * image:
 * dimmingFactor:
 */
void dim_effect(Image* image, const uint8_t dimmingFactor);

/* min_val()
 * ---------
 * val:
 * contrastFactor:
 * min:
 * max:
 *
 * Returns:
 */
uint8_t min_val(uint8_t val, const uint8_t contrastFactor, const uint8_t min,
        const uint8_t max);

#endif
