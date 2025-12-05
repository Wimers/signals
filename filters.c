// Included Libraries
#include <stdlib.h>
#include <string.h>
#include "fileParsing.h"
#include "filters.h"
#include "main.h"

const char* const fileDimensionMismatchMessage
        = "File dimension mismatch: \"%dx%d\" is not \"%dx%d\"\n";
const char* const imageBoundsMessage = "Image bounds (%dx%d)\n";

void invert_colours(Image* image)
{
    // For each row
    for (int y = 0; y < image->height; y++) {
        Pixel* row = image->pixels[y];

        // For each pixel in row
        for (int x = 0; x < image->width; x++) {
            Pixel* pixel = &(row[x]);

            pixel->red = UINT8_MAX - pixel->red;
            pixel->green = UINT8_MAX - pixel->green;
            pixel->blue = UINT8_MAX - pixel->blue;
        }
    }
}

void filter_red(Image* image)
{
    // For each row
    for (int y = 0; y < image->height; y++) {
        Pixel* row = (image->pixels)[y];

        // For each pixel in row
        for (int x = 0; x < image->width; x++) {

            // Disable the red component of the pixel
            row[x].red = 0;
        }
    }
}

void filter_green(Image* image)
{
    // For each row
    for (int y = 0; y < image->height; y++) {
        Pixel* row = (image->pixels)[y];

        // For each pixel in row
        for (int x = 0; x < image->width; x++) {

            // Disable the red component of the pixel
            row[x].green = 0;
        }
    }
}

void filter_blue(Image* image)
{
    // For each row
    for (int y = 0; y < image->height; y++) {
        Pixel* row = (image->pixels)[y];

        // For each pixel in row
        for (int x = 0; x < image->width; x++) {

            // Disable the red component of the pixel
            row[x].blue = 0;
        }
    }
}

void gray_filter(Image* image)
{
    // For each row
    for (int y = 0; y < image->height; y++) {
        Pixel* row = image->pixels[y];

        // For each pixel in row
        for (int x = 0; x < image->width; x++) {
            Pixel* pixel = &(row[x]);

            // Calculate gray scaled value using weighted values based on how
            // sensitive the human eye is to different wavelengths of light.
            const uint32_t temp = (uint32_t)(GS_RED_MAP * pixel->red
                    + GS_GREEN_MAP * pixel->green + GS_BLUE_MAP * pixel->blue);

            const uint8_t grayScaled
                    = (uint8_t)(temp >> GS_PIXEL_SCALING_FACTOR);

            // Assign value to each pixel
            pixel->red = pixel->green = pixel->blue = grayScaled;
        }
    }
}

void average_pixels(Image* image)
{
    // For each row
    for (int y = 0; y < image->height; y++) {
        Pixel* row = (image->pixels)[y];

        // For each pixel in row
        for (int x = 0; x < image->width; x++) {
            Pixel* pixel = &(row[x]);

            uint8_t brightness
                    = (uint8_t)((pixel->red + pixel->green + pixel->blue) / 3);
            pixel->blue = brightness;
            pixel->green = brightness;
            pixel->red = brightness;
        }
    }
}

void brightness_cap_filter(Image* image, const uint8_t maxBrightness)
{
    // For each row
    for (int y = 0; y < image->height; y++) {
        Pixel* row = image->pixels[y];

        // For each pixel in row
        for (int x = 0; x < image->width; x++) {
            Pixel* pixel = &(row[x]);

            if (pixel->blue > maxBrightness) {

                // Disable blue component of pixel
                pixel->blue = 0;
            }

            if (pixel->green > maxBrightness) {

                // Disable green component of pixel
                pixel->green = 0;
            }

            if (pixel->red > maxBrightness) {

                // Disable red component of pixel
                pixel->red = 0;
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

    // For each row
    for (int y = 0; y < height; y++) {

        Pixel* pRow = (primary->pixels)[y];
        Pixel* sRow = (secondary->pixels)[y];

        // For each pixel in row
        for (int x = 0; x < width; x++) {
            Pixel* pPixel = &(pRow[x]);
            Pixel* sPixel = &(sRow[x]);

            uint8_t newRed = (uint8_t)((pPixel->red + sPixel->red) >> 1);
            pPixel->red = newRed;

            uint8_t newGreen = (uint8_t)((pPixel->green + sPixel->green) >> 1);
            pPixel->green = newGreen;

            uint8_t newBlue = (uint8_t)((pPixel->blue + sPixel->blue) >> 1);
            pPixel->blue = newBlue;
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

    Pixel* rowCopy = malloc((size_t)image->width * sizeof(Pixel));
    if (rowCopy == NULL) {
        return -1; // Add unique exit code
    }

    // For each row
    for (int y = 0; y < image->height; y++) {

        Pixel* row = (image->pixels)[y];
        memcpy(rowCopy, row, (size_t)(image->width) * sizeof(Pixel));

        // For each pixel in row
        for (int x = 0; x < image->width; x++) {

            const int accessRedRegion = x + glitchOffset;
            if ((accessRedRegion < 0) || (accessRedRegion >= image->width)) {
                ;
            } else {
                row[x].red = rowCopy[accessRedRegion].red;
            }

            const int accessBlueRegion = x - glitchOffset;
            if ((accessBlueRegion < 0) || (accessBlueRegion >= image->width)) {
                ;
            } else {
                row[x].blue = rowCopy[accessBlueRegion].blue;
            }
        }
    }

    free(rowCopy);
    return EXIT_SUCCESS;
}

int verify_offset_bounds(Image* image, const int32_t offset) // FIX
{
    if (offset >= image->width) {
        fputs(glitchUsageMessage, stderr);
        fprintf(stderr, imageBoundsMessage, image->width, image->height);
        fputs(glitchOffsetValMessage, stderr);
        fprintf(stderr, gotIntMessage, offset);
        return -1;
    }

    return EXIT_SUCCESS;
}

void contrast_effect(Image* image, const uint8_t contrastFactor,
        const uint8_t min, const uint8_t max)
{
    // For each row
    for (int y = 0; y < image->height; y++) {
        Pixel* row = (image->pixels)[y];

        // For each pixel in row
        for (int x = 0; x < image->width; x++) {
            Pixel* pixel = &(row[x]);

            // Apply contrast filter to each colour
            min_val(&(pixel->blue), contrastFactor, min, max);
            min_val(&(pixel->green), contrastFactor, min, max);
            min_val(&(pixel->red), contrastFactor, min, max);
        }
    }
}

void dim_effect(Image* image, const uint8_t dimmingFactor)
{
    // For each row
    for (int y = 0; y < image->height; y++) {
        Pixel* row = (image->pixels)[y];

        // For each pixel in row
        for (int x = 0; x < image->width; x++) {
            Pixel* pixel = &(row[x]);

            if (pixel->blue <= dimmingFactor) {
                pixel->blue = 0;
            } else {
                pixel->blue -= dimmingFactor;
            }

            if (pixel->green <= dimmingFactor) {
                pixel->green = 0;
            } else {
                pixel->green -= dimmingFactor;
            }

            if (pixel->red <= dimmingFactor) {
                pixel->red = 0;
            } else {
                pixel->red -= dimmingFactor;
            }
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
