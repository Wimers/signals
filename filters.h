// #ifndef FILTERS_H
// #define FILTERS_H

#include "fileParsing.h"
#include <stdint.h>

void filter_invert_colours(Image* image);
void filter_blue(Image* image);
void filter_red(Image* image);
void filter_green(Image* image);
void gray_filter(Image* image);
void brightness_filter(Image* image);
void brightness_cap_filter(Image* image, const uint8_t maxBrightness);
void combine_images(Image* restrict primary, const Image* restrict secondary);
void glitch_effect(Image* image, const int32_t glitchOffset);

// #endif
