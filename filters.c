// Included Libraries
#include <stdlib.h>
#include "fileParsing.h"
#include "filters.h"

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

            uint8_t grayScaled = (uint8_t)(0.299 * image->pixels[y][x].red
                    + 0.587 * image->pixels[y][x].green
                    + 0.114 * image->pixels[y][x].blue);
            image->pixels[y][x].blue = grayScaled;
            image->pixels[y][x].green = grayScaled;
            image->pixels[y][x].red = grayScaled;
        }
    }
}

void brightness_filter(Image* image)
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

void brightness_cap_filter(Image* image, uint8_t maxBrightness)
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

void combine_images(Image* primary, Image* secondary)
{
    uint16_t height = primary->height;
    uint16_t width = primary->width;

    if ((secondary->height != height) || (secondary->width != width)) {
        exit(EXIT_OUT_OF_BOUNDS);
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
