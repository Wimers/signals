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
            image->pixels[y][x].red = UINT8_MAX - image->pixels[y][x].red;
            image->pixels[y][x].green = UINT8_MAX - image->pixels[y][x].green;
            image->pixels[y][x].blue = UINT8_MAX - image->pixels[y][x].blue;
        }
    }
}

void filter_red(Image* image)
{
    // For each row
    for (int y = 0; y < image->height; y++) {

        // For each pixel
        for (int x = 0; x < image->width; x++) {

            // Disable the red component of the pixel
            image->pixels[y][x].red = 0;
        }
    }
}

void filter_green(Image* image)
{
    // For each row
    for (int y = 0; y < image->height; y++) {

        // For each pixel
        for (int x = 0; x < image->width; x++) {

            // Disable the red component of the pixel
            image->pixels[y][x].green = 0;
        }
    }
}

void filter_blue(Image* image)
{
    // For each row
    for (int y = 0; y < image->height; y++) {

        // For each pixel
        for (int x = 0; x < image->width; x++) {

            // Disable the red component of the pixel
            image->pixels[y][x].blue = 0;
        }
    }
}

void gray_filter(Image* image)
{
    // For each row
    for (int y = 0; y < image->height; y++) {

        // For each pixel
        for (int x = 0; x < image->width; x++) {

            // Calculate gray scaled value using weighted values based on how
            // sensitive the human eye is to different wavelengths of light.
            const uint32_t temp
                    = (uint32_t)(GS_RED_MAP * image->pixels[y][x].red
                            + GS_GREEN_MAP * image->pixels[y][x].green
                            + GS_BLUE_MAP * image->pixels[y][x].blue);

            const uint8_t grayScaled
                    = (uint8_t)(temp >> GS_PIXEL_SCALING_FACTOR);

            // Assign value to each pixel
            image->pixels[y][x].red = image->pixels[y][x].green
                    = image->pixels[y][x].blue = grayScaled;
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
    // For each row
    for (int y = 0; y < image->height; y++) {

        // For each pixel in row
        for (int x = 0; x < image->width; x++) {

            if (image->pixels[y][x].blue > maxBrightness) {

                // Disable blue component of pixel
                image->pixels[y][x].blue = 0;
            }

            if (image->pixels[y][x].green > maxBrightness) {

                // Disable green component of pixel
                image->pixels[y][x].green = 0;
            }

            if (image->pixels[y][x].red > maxBrightness) {

                // Disable red component of pixel
                image->pixels[y][x].red = 0;
            }
        }
    }
}

int combine_images(Image* restrict primary, const Image* restrict secondary)
{
    const int32_t height = primary->height;
    const int32_t width = primary->width;

    if ((secondary->height != height) || (secondary->width != width)) {
        fprintf(stderr, fileDimensionMismatchMessage, height, width,
                secondary->height, secondary->width);
        return EXIT_OUT_OF_BOUNDS;
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

    return EXIT_SUCCESS;
}

int glitch_effect(Image* image, const int32_t glitchOffset)
{
    // Check if offset is out of image bounds
    if (verify_offset_bounds(image, glitchOffset) == -1) {
        return -1;
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
    return EXIT_SUCCESS;
}

int verify_offset_bounds(Image* image, const int32_t offset) // FIX
{
    if (offset >= image->width) {
        fputs(glitchUsageMessage, stderr);
        fprintf(stderr, "Image bounds (%dx%d)\n", image->width, image->height);
        fputs(glitchOffsetValMessage, stderr);
        fprintf(stderr, gotIntMessage, offset);
        free_image(image);
        return -1;
    }

    return EXIT_SUCCESS;
}

void contrast_effect(Image* image, const uint8_t contrastFactor,
        const uint8_t min, const uint8_t max)
{
    // For each row
    for (int y = 0; y < image->height; y++) {

        // For each pixel
        for (int x = 0; x < image->width; x++) {

            // Apply contrast filter to each colour
            min_val(&(image->pixels[y][x].blue), contrastFactor, min, max);
            min_val(&(image->pixels[y][x].green), contrastFactor, min, max);
            min_val(&(image->pixels[y][x].red), contrastFactor, min, max);
        }
    }
}

void min_val(uint8_t* val, const uint8_t contrastFactor, const uint8_t min,
        const uint8_t max)
{
    if (*val >= max) {
        const int sumMax = *val + contrastFactor;
        if (sumMax <= UINT8_MAX) {
            *val = (uint8_t)sumMax;
            return;
        }

        *val = (uint8_t)UINT8_MAX;
        return;
    }

    if (*val <= min) {
        const int sumMin = *val - contrastFactor;
        if (sumMin <= 0) {
            *val = (uint8_t)0;
            return;
        }

        if (sumMin >= UINT8_MAX) {
            *val = (uint8_t)UINT8_MAX;
            return;
        }

        *val = (uint8_t)sumMin;
        return;
    }
}

void dim_effect(Image* image, const uint8_t dimmingFactor)
{
    // For each row
    for (int y = 0; y < image->height; y++) {

        // For each pixel in row
        for (int x = 0; x < image->width; x++) {

            if (image->pixels[y][x].blue <= dimmingFactor) {
                image->pixels[y][x].blue = 0;
            } else {
                image->pixels[y][x].blue -= dimmingFactor;
            }

            if (image->pixels[y][x].green <= dimmingFactor) {
                image->pixels[y][x].green = 0;
            } else {
                image->pixels[y][x].green -= dimmingFactor;
            }

            if (image->pixels[y][x].red <= dimmingFactor) {
                image->pixels[y][x].red = 0;
            } else {
                image->pixels[y][x].red -= dimmingFactor;
            }
        }
    }
}
