// #ifndef FILTERS_H
// #define FILTERS_H

#include "fileParsing.h"
#include <stdint.h>

// Constants to best map RGB values to grayscale
#define GS_RED_MAP 0.299
#define GS_GREEN_MAP 0.587
#define GS_BLUE_MAP 0.114

void filter_invert_colours(Image* image);
void filter_blue(Image* image);
void filter_red(Image* image);
void filter_green(Image* image);
void gray_filter(Image* image);
void average_pixels(Image* image);
void brightness_cap_filter(Image* image, const uint8_t maxBrightness);
void combine_images(Image* restrict primary, const Image* restrict secondary);
void glitch_effect(Image* image, const int32_t glitchOffset);

// #endif
