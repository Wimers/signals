// Included Libraries
#include <stdlib.h>
#include <string.h>
#include "fileParsing.h"
#include "filters.h"
#include "main.h"

const char* const fileDimensionMismatchMessage
        = "File dimension mismatch: \"%zux%zu\" is not \"%zux%zu\"\n";
const char* const imageBoundsMessage = "Image bounds (%zux%zu)\n";

void invert_colours(Image* image)
{
    // For each row
    for (size_t y = 0; y < image->height; y++) {
        Pixel* row = (image->pixels)[y];

        // For each pixel in row
        for (size_t x = 0; x < image->width; x++) {
            Pixel* pixel = &(row[x]);

            // Invert pixel colour value
            pixel->red = (uint8_t)(UINT8_MAX - pixel->red);
            pixel->green = (uint8_t)(UINT8_MAX - pixel->green);
            pixel->blue = (uint8_t)(UINT8_MAX - pixel->blue);
        }
    }
}

void filter_red(Image* image)
{
    // For each row
    for (size_t y = 0; y < image->height; y++) {
        Pixel* row = (image->pixels)[y];

        // For each pixel in row
        for (size_t x = 0; x < image->width; x++) {

            // Disable the red component of the pixel
            row[x].red = 0;
        }
    }
}

void filter_green(Image* image)
{
    // For each row
    for (size_t y = 0; y < image->height; y++) {
        Pixel* row = (image->pixels)[y];

        // For each pixel in row
        for (size_t x = 0; x < image->width; x++) {

            // Disable the red component of the pixel
            row[x].green = 0;
        }
    }
}

void filter_blue(Image* image)
{
    // For each row
    for (size_t y = 0; y < image->height; y++) {
        Pixel* row = (image->pixels)[y];

        // For each pixel in row
        for (size_t x = 0; x < image->width; x++) {

            // Disable the red component of the pixel
            row[x].blue = 0;
        }
    }
}

void filter_red_green(Image* image)
{
    // For each row
    for (size_t y = 0; y < image->height; y++) {
        Pixel* row = (image->pixels)[y];

        // For each pixel in row
        for (size_t x = 0; x < image->width; x++) {

            // Disable the red and green components of the pixel
            row[x].red = 0;
            row[x].green = 0;
        }
    }
}

void filter_red_blue(Image* image)
{
    // For each row
    for (size_t y = 0; y < image->height; y++) {
        Pixel* row = (image->pixels)[y];

        // For each pixel in row
        for (size_t x = 0; x < image->width; x++) {

            // Disable the red and blue components of the pixel
            row[x].red = 0;
            row[x].blue = 0;
        }
    }
}

void filter_green_blue(Image* image)
{
    // For each row
    for (size_t y = 0; y < image->height; y++) {
        Pixel* row = (image->pixels)[y];

        // For each pixel in row
        for (size_t x = 0; x < image->width; x++) {

            // Disable the green and blue components of the pixel
            row[x].blue = 0;
            row[x].green = 0;
        }
    }
}

void filter_all(Image* image)
{
    // For each row
    for (size_t y = 0; y < image->height; y++) {
        Pixel* row = (image->pixels)[y];

        // For each pixel in row
        for (size_t x = 0; x < image->width; x++) {

            // Set the pixel to pure black
            memset(&(row[x]), 0, sizeof(Pixel));
        }
    }
}

void gray_filter(Image* image)
{
    // For each row
    for (size_t y = 0; y < image->height; y++) {
        Pixel* row = image->pixels[y];

        // For each pixel in row
        for (size_t x = 0; x < image->width; x++) {
            Pixel* pixel = &(row[x]);

            // Calculate gray scaled value
            const uint8_t grayScaled = calc_pixel_grayscale(pixel);

            // Assign value to each pixel
            pixel->red = pixel->green = pixel->blue = grayScaled;
        }
    }
}

void average_pixels(Image* image)
{
    // For each row
    for (size_t y = 0; y < image->height; y++) {
        Pixel* row = (image->pixels)[y];

        // For each pixel in row
        for (size_t x = 0; x < image->width; x++) {
            Pixel* pixel = &(row[x]);

            // Calculates the mean value of the pixel
            const uint8_t brightness = calc_pixel_average(pixel);

            pixel->blue = brightness;
            pixel->green = brightness;
            pixel->red = brightness;
        }
    }
}

void brightness_cap_filter(Image* image, const uint8_t maxBrightness)
{
    // For each row
    for (size_t y = 0; y < image->height; y++) {
        Pixel* row = image->pixels[y];

        // For each pixel in row
        for (size_t x = 0; x < image->width; x++) {
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
    const size_t height = primary->height;
    const size_t width = primary->width;

    // Check images share the same physical dimensions
    if ((secondary->height != height) || (secondary->width != width)) {

        // Print error message
        fprintf(stderr, fileDimensionMismatchMessage, height, width,
                secondary->height, secondary->width);
        return EXIT_OUT_OF_BOUNDS;
    }

    // For each row
    for (size_t y = 0; y < height; y++) {

        // For reduced cpu cycles
        Pixel* pRow = (primary->pixels)[y];
        Pixel* sRow = (secondary->pixels)[y];

        // For each pixel in row
        for (size_t x = 0; x < width; x++) {

            // For reduced cpu cycles
            Pixel* pPixel = &(pRow[x]);
            Pixel* sPixel = &(sRow[x]);

            // Average the each colour value from each image and update value in
            // primary image.

            const uint8_t newRed = (uint8_t)((pPixel->red + sPixel->red) >> 1);
            pPixel->red = newRed;

            const uint8_t newGreen
                    = (uint8_t)((pPixel->green + sPixel->green) >> 1);
            pPixel->green = newGreen;

            const uint8_t newBlue
                    = (uint8_t)((pPixel->blue + sPixel->blue) >> 1);
            pPixel->blue = newBlue;
        }
    }

    return EXIT_SUCCESS;
}

int glitch_effect(Image* image, const size_t glitchOffset)
{
    // Check if offset is out of image bounds
    if (verify_offset_bounds(image, glitchOffset) == -1) {
        return -1;
    }

    Pixel* rowCopy = malloc(image->width * sizeof(Pixel));
    if (rowCopy == NULL) {
        // Malloc failed
        return -1; // FIX Add unique exit code
    }

    // For each row
    for (size_t y = 0; y < image->height; y++) {
        Pixel* row = (image->pixels)[y];

        // Copy data from the row to allow glitch pixel values to be calculated
        // based on original image appearance.
        memcpy(rowCopy, row, image->width * sizeof(Pixel));

        // For each pixel in row
        for (size_t x = 0; x < image->width; x++) {

            // Update pixel value if data access region is within image bounds,
            // else set component to zero.
            const size_t accessRedRegion = x + glitchOffset;
            (accessRedRegion < image->width)
                    ? (row[x].red = rowCopy[accessRedRegion].red)
                    : 0;

            const size_t accessBlueRegion = x - glitchOffset;
            (accessBlueRegion < image->width)
                    ? (row[x].blue = rowCopy[accessBlueRegion].blue)
                    : 0;
        }
    }

    // Free temp memory and exit
    free(rowCopy);
    return EXIT_SUCCESS;
}

int verify_offset_bounds(Image* image, const size_t offset) // FIX
{
    if (offset >= image->width) {
        fputs(glitchUsageMessage, stderr);
        fprintf(stderr, imageBoundsMessage, image->width, image->height);
        fputs(glitchOffsetValMessage, stderr);
        fprintf(stderr, "got \"%zu\".\n", offset); // FIX
        return -1;
    }

    return EXIT_SUCCESS;
}

void contrast_effect(Image* image, const uint8_t contrastFactor,
        const uint8_t min, const uint8_t max)
{
    // Create a lookup table mapping input -> output pixels based on the
    // contrast factor, and min and max values.
    uint8_t lookupTable[UINT8_MAX + 1] = {0};

    for (int i = 0; i <= UINT8_MAX; i++) {
        lookupTable[i]
                = contrast_effect_val((uint8_t)i, contrastFactor, min, max);
    }

    // For each row
    for (size_t y = 0; y < image->height; y++) {
        Pixel* row = (image->pixels)[y];

        // For each pixel in row
        for (size_t x = 0; x < image->width; x++) {
            Pixel* pixel = &(row[x]);

            // Apply contrast filter to each colour
            pixel->blue = lookupTable[pixel->blue];
            pixel->green = lookupTable[pixel->green];
            pixel->red = lookupTable[pixel->red];
        }
    }
}

void dim_effect(Image* image, const uint8_t dimmingFactor)
{
    // For each row
    for (size_t y = 0; y < image->height; y++) {
        Pixel* row = (image->pixels)[y];

        // For each pixel in row
        for (size_t x = 0; x < image->width; x++) {
            Pixel* pixel = &(row[x]);

            // Reduce value of each pixel component by the dimming factor.
            // Zero is the minimum value components can be reduced to.
            pixel->blue = (pixel->blue <= dimmingFactor)
                    ? (0)
                    : ((uint8_t)(pixel->blue - dimmingFactor));

            pixel->green = (pixel->green <= dimmingFactor)
                    ? (0)
                    : ((uint8_t)(pixel->green - dimmingFactor));

            pixel->red = (pixel->red <= dimmingFactor)
                    ? (0)
                    : ((uint8_t)(pixel->red - dimmingFactor));
        }
    }
}

uint8_t contrast_effect_val(const uint8_t val, const uint8_t contrastFactor,
        const uint8_t min, const uint8_t max)
{
    if (val >= max) {
        const int sumMax = val + contrastFactor;
        return (sumMax <= UINT8_MAX) ? ((uint8_t)sumMax) : ((uint8_t)UINT8_MAX);
    }

    if (val <= min) {
        const int sumMin = val - contrastFactor;

        if (sumMin <= 0) {
            return (uint8_t)0;
        }
        return (sumMin < UINT8_MAX) ? ((uint8_t)sumMin) : ((uint8_t)UINT8_MAX);
    }

    return val;
}

void swap_red_blue(Image* image)
{
    // For each row
    for (size_t y = 0; y < image->height; y++) {
        Pixel* row = (image->pixels)[y];

        // For each pixel in row
        for (size_t x = 0; x < image->width; x++) {
            Pixel* pixel = &(row[x]);

            // Swap red and blue values
            const uint8_t temp = pixel->red;
            pixel->red = pixel->blue;
            pixel->blue = temp;
        }
    }
}

void reverse_image(Image* image)
{
    for (size_t y = 0; y < image->height; y++) {
        Pixel* row = (image->pixels)[y];

        // For each pixel in row
        for (size_t x = 0; x < (image->width >> 1); x++) {

            const size_t last = image->width - x - 1;
            Pixel temp = row[x];
            row[x] = row[last];
            row[last] = temp;
        }
    }
}

Image* rotate_image(Image* image, const long nRotations)
{
    // If rotation results in original image
    if ((nRotations % 4) == 0) {
        return image;
    }

    if ((nRotations % 4) == 1) {
        Image* rotated = create_image((int32_t)(image->height),
                (int32_t)(image->width)); // FIX type casts

        if (rotated == NULL) {
            return NULL;
        }

        // For each row of pixels
        for (size_t y = 0; y < image->height; y++) {
            Pixel* row = (image->pixels)[y];

            for (size_t x = 0; x < image->width; x++) {
                (rotated->pixels)[x][y] = row[x];
            }
        }

        flip_image(rotated);
        return rotated;
    }

    if ((nRotations % 4) == 2) {
        flip_image(image);
        reverse_image(image);
        return image;
    }

    if ((nRotations % 4) == 3) {
        Image* rotated = create_image((int32_t)(image->height),
                (int32_t)(image->width)); // FIX type casts

        if (rotated == NULL) {
            return NULL;
        }

        // For each row of pixels
        for (size_t y = 0; y < image->height; y++) {
            Pixel* row = (image->pixels)[y];

            for (size_t x = 0; x < image->width; x++) {
                (rotated->pixels)[x][y] = row[x];
            }
        }

        reverse_image(rotated);
        return rotated;
    }

    // Cannot happen
    return NULL;
}
