#ifndef PIXELS_H
#define PIXELS_H

#include <stdint.h>
#include <stddef.h>

typedef struct __attribute__((packed)) {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
} Pixel;

typedef struct {
    size_t width;
    size_t height;
    Pixel* pixelData;
} Image;

#endif
