// #ifndef FILTERS_H
// #define FILTERS_H

#include "fileParsing.h"
void filter_invert_colours(Image* image);
void filter_blue(Image* image);
void filter_red(Image* image);
void filter_green(Image* image);
void gray_filter(Image* image);
void brightness_filter(Image* image);
void brightness_cap_filter(Image* image, uint8_t maxBrightness);

// #endif
