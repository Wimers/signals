// Included Libraries
#include <stdlib.h>
#include <string.h>
#include "fileParsing.h"
#include "filters.h"
#include "main.h"

const char* const fileDimensionMismatchMessage
        = "File dimension mismatch: \"%dx%d\" is not \"%dx%d\"\n";

void filter_invert_colours(Image* image)
{
    for (int y = 0; y < image->height; y++) {
        for (int x = 0; x < image->width; x++) {
            image->pixels[y][x].red = 255 - image->pixels[y][x].red;
            image->pixels[y][x].green = 255 - image->pixels[y][x].green;
            image->pixels[y][x].blue = 255 - image->pixels[y][x].blue;
        }
    }
}

void filter_red(Image* image)
{
    for (int y = 0; y < image->height; y++) {
        for (int x = 0; x < image->width; x++) {
            image->pixels[y][x].red = 0;
        }
    }
}

void filter_green(Image* image)
{
    for (int y = 0; y < image->height; y++) {
        for (int x = 0; x < image->width; x++) {
            image->pixels[y][x].green = 0;
        }
    }
}

void filter_blue(Image* image)
{
    for (int y = 0; y < image->height; y++) {
        for (int x = 0; x < image->width; x++) {
            image->pixels[y][x].blue = 0;
        }
    }
}

void gray_filter(Image* image)
{
    for (int y = 0; y < image->height; y++) {
        for (int x = 0; x < image->width; x++) {

            uint8_t grayScaled = (uint8_t)(GS_RED_MAP * image->pixels[y][x].red
                    + GS_GREEN_MAP * image->pixels[y][x].green
                    + GS_BLUE_MAP * image->pixels[y][x].blue);
            image->pixels[y][x].blue = grayScaled;
            image->pixels[y][x].green = grayScaled;
            image->pixels[y][x].red = grayScaled;
        }
    }
}

void average_pixels(Image* image)
{
    for (int y = 0; y < image->height; y++) {
        for (int x = 0; x < image->width; x++) {

            uint8_t brightness = (uint8_t)((image->pixels[y][x].red
                                                   + image->pixels[y][x].green
                                                   + image->pixels[y][x].blue)
                    / 3);
            image->pixels[y][x].blue = brightness;
            image->pixels[y][x].green = brightness;
            image->pixels[y][x].red = brightness;
        }
    }
}

void brightness_cap_filter(Image* image, const uint8_t maxBrightness)
{
    for (int y = 0; y < image->height; y++) {
        for (int x = 0; x < image->width; x++) {

            if (image->pixels[y][x].blue > maxBrightness) {
                image->pixels[y][x].blue = 0;
            }

            if (image->pixels[y][x].green > maxBrightness) {
                image->pixels[y][x].green = 0;
            }

            if (image->pixels[y][x].red > maxBrightness) {
                image->pixels[y][x].red = 0;
            }
        }
    }
}

void combine_images(Image* restrict primary, const Image* restrict secondary)
{
    const int32_t height = primary->height;
    const int32_t width = primary->width;

    if ((secondary->height != height) || (secondary->width != width)) {
        fprintf(stderr, fileDimensionMismatchMessage, height, width,
                secondary->height, secondary->width);
        exit(EXIT_OUT_OF_BOUNDS); // Results in memory leak I believe
    }

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            uint8_t newRed = (primary->pixels[y][x].red) / 2
                    + (secondary->pixels[y][x].red / 2);
            primary->pixels[y][x].red = newRed;

            uint8_t newGreen = (primary->pixels[y][x].green) / 2
                    + (secondary->pixels[y][x].green / 2);
            primary->pixels[y][x].green = newGreen;

            uint8_t newBlue = (primary->pixels[y][x].blue) / 2
                    + (secondary->pixels[y][x].blue / 2);
            primary->pixels[y][x].blue = newBlue;
        }
    }
}

void glitch_effect(Image* image, const int32_t glitchOffset)
{
    // Check if offset is out of image bounds
    if (glitchOffset >= (image->width)) {
        fputs(glitchUsageMessage, stderr);
        fprintf(stderr, "Image bounds (%dx%d)\n", image->width, image->height);
        fputs(glitchOffsetValMessage, stderr);
        fprintf(stderr, gotIntMessage, glitchOffset);
        exit(EXIT_OUT_OF_BOUNDS); // Results in a memory leak I believe
    }

    Image* imageCopy = create_image(image->width, image->height);

    for (int y = 0; y < image->height; y++) {
        memcpy(imageCopy->pixels[y], image->pixels[y],
                image->width * sizeof(Pixel));

        for (int x = 0; x < image->width; x++) {
            int accessRedRegion = x + glitchOffset;
            if ((accessRedRegion < 0) || (accessRedRegion > image->width)) {
                ;
            } else {
                image->pixels[y][x].red
                        = imageCopy->pixels[y][accessRedRegion].red;
            }

            int accessBlueRegion = x - glitchOffset;

            if ((accessBlueRegion < 0) || (accessBlueRegion > image->width)) {
                ;
            } else {
                image->pixels[y][x].red
                        = imageCopy->pixels[y][accessBlueRegion].red;
            }
        }
    }

    free_image(imageCopy);
}
