#ifndef FILTERS_H
#define FILTERS_H

#include "fileParsing.h"
#include <stdint.h>

// Constants to best map RGB values to grayscale
// Each multiplied by 1000 to avoid floats
#define GS_RED_MAP 299
#define GS_GREEN_MAP 587
#define GS_BLUE_MAP 114

// Used for division by 1024 (more efficient than dividing by 1000)
#define GS_PIXEL_SCALING_FACTOR 10

extern const char* const fileDimensionMismatchMessage;

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
 */
void min_val(uint8_t* val, const uint8_t contrastFactor, const uint8_t min,
        const uint8_t max);

#endif
