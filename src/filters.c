// Included Libraries
#include <stdlib.h>
#include <string.h>
#include "fileParsing.h"
#include "filters.h"
#include "main.h"
#include "imageEditing.h"

constexpr char fileDimensionMismatchMessage[]
        = "File dimension mismatch: \"%zux%zu\" is not \"%zux%zu\"\n";
constexpr char imageBoundsMessage[] = "Image bounds (%zux%zu)\n";

static inline uint8_t clamp_floor_u8(const uint8_t val, const uint8_t sub)
{
    return (uint8_t)((val > sub) ? (val - sub) : (0));
}

static inline uint8_t clamp_ceil_u8(const uint8_t val, const uint8_t add)
{
    const int sum = val + add;
    return (uint8_t)((sum >= UINT8_MAX) ? (UINT8_MAX) : (sum));
}

static inline uint8_t average_channel(const uint8_t a, const uint8_t b)
{
    return (uint8_t)((a + b) >> 1);
}

void invert_colours(Image* image)
{
    FX_TEMPLATE(image,
            { // Invert pixel colour value
                pixel->blue ^= -1;
                pixel->green ^= -1;
                pixel->red ^= -1;
            });
}

void filter_red(Image* image)
{
    FX_TEMPLATE(image,
            { // Disable the red component of the pixel
                pixel->red = 0;
            });
}

void filter_green(Image* image)
{
    FX_TEMPLATE(image,
            { // Disable the green component of the pixel
                pixel->green = 0;
            });
}

void filter_blue(Image* image)
{
    FX_TEMPLATE(image,
            { // Disable the blue component of the pixel
                pixel->blue = 0;
            });
}

void filter_red_green(Image* image)
{
    FX_TEMPLATE(image,
            { // Disable the red and green components of the pixel
                pixel->green = 0;
                pixel->red = 0;
            });
}

void filter_red_blue(Image* image)
{
    FX_TEMPLATE(image,
            { // Disable the red and blue components of the pixel
                pixel->blue = 0;
                pixel->red = 0;
            });
}

void filter_green_blue(Image* image)
{
    FX_TEMPLATE(image,
            { // Disable the green and blue components of the pixel
                pixel->blue = 0;
                pixel->green = 0;
            });
}

void filter_all(Image* image)
{
    // Zero everything
    memset(image->pixelData, 0, sizeof(Pixel) * image->width * image->height);
}

void gray_filter(Image* image)
{
    FX_TEMPLATE(image, { // Calculate gray scaled value
        const uint8_t grayScaled = calc_pixel_grayscale(pixel);

        // Assign value to each pixel
        pixel->red = pixel->green = pixel->blue = grayScaled;
    });
}

void average_pixels(Image* image)
{
    FX_TEMPLATE(image, { // Calculates the mean value of the pixel
        const uint8_t brightness = calc_pixel_average(pixel);
        pixel->red = pixel->green = pixel->blue = brightness;
    });
}

void brightness_cut_filter(Image* image, const uint8_t cutoff)
{
    FX_TEMPLATE(image, { // Disable colour
        pixel->blue = (uint8_t)(pixel->blue * (pixel->blue <= cutoff));
        pixel->green = (uint8_t)(pixel->green * (pixel->green <= cutoff));
        pixel->red = (uint8_t)(pixel->red * (pixel->red <= cutoff));
    });
}

[[nodiscard]] int combine_images(
        Image* restrict primary, const Image* restrict secondary)
{
    const size_t height = primary->height;
    const size_t width = primary->width;

    // Check images share the same physical dimensions
    if ((secondary->height != height) || (secondary->width != width)) {

        // Print error message
        fprintf(stderr, fileDimensionMismatchMessage, height, width,
                secondary->height, secondary->width);
        fflush(stderr);
        return EXIT_OUT_OF_BOUNDS;
    }

    // For each row
    for (size_t y = 0; y < height; y++) {
        size_t rowOffset = y * primary->width;

        // For each pixel in row
        for (size_t x = 0; x < width; x++) {

            // For reduced cpu cycles
            Pixel* pPixel = get_pixel_fast(primary, x, rowOffset);
            Pixel* sPixel = get_pixel_fast(secondary, x, rowOffset);

            // Average the each colour value from each image and update
            // value in primary image.
            pPixel->blue = average_channel(pPixel->blue, sPixel->blue);
            pPixel->green = average_channel(pPixel->green, sPixel->green);
            pPixel->red = average_channel(pPixel->red, sPixel->red);
        }
    }

    return EXIT_SUCCESS;
}

[[nodiscard]] int merge_images(
        Image* restrict primary, const Image* restrict secondary)
{
    const size_t height = primary->height;
    const size_t width = primary->width;

    // Check images share the same physical dimensions
    if ((secondary->height != height) || (secondary->width != width)) {

        // Print error message
        fprintf(stderr, fileDimensionMismatchMessage, height, width,
                secondary->height, secondary->width);
        fflush(stderr);
        return EXIT_OUT_OF_BOUNDS;
    }

    // For each row
    for (size_t y = 0; y < height; y++) {
        size_t rowOffset = y * primary->width;

        // For each pixel in row
        for (size_t x = 0; x < width; x++) {

            // For reduced cpu cycles
            Pixel* pPixel = get_pixel_fast(primary, x, rowOffset);
            Pixel* sPixel = get_pixel_fast(secondary, x, rowOffset);

            // Average the each colour value from each image and update
            // value in primary image.
            pPixel->blue = clamp_ceil_u8(pPixel->blue, sPixel->blue);
            pPixel->green = clamp_ceil_u8(pPixel->green, sPixel->green);
            pPixel->red = clamp_ceil_u8(pPixel->red, sPixel->red);
        }
    }

    return EXIT_SUCCESS;
}

[[nodiscard]] int glitch_effect(Image* image, const size_t glitchOffset)
{
    // Check if offset is out of image bounds
    if (verify_offset_bounds(image, glitchOffset) == -1) {
        return -1;
    }

    const size_t rowSize = image->width * sizeof(Pixel);

    Pixel* rowCopy = malloc(rowSize);
    if (rowCopy == NULL) {
        // Malloc failed
        return -1; // FIX Add unique exit code
    }

    // For each row
    for (size_t y = 0; y < image->height; y++) {
        size_t rowOffset = image->width * y;
        Pixel* row = &((image->pixelData)[rowOffset]);

        // Copy data from the row to allow glitch pixel values to be
        // calculated based on original image appearance.
        memcpy(rowCopy, row, rowSize);

        // For each pixel in row
        for (size_t x = 0; x < image->width; x++) {
            Pixel* pixel = get_pixel_fast(image, x, rowOffset);

            // Update pixel value if data access region is within image
            // bounds, else set component to zero.
            const size_t accessRedRegion = x + glitchOffset;
            (accessRedRegion < image->width)
                    ? (pixel->red = rowCopy[accessRedRegion].red)
                    : 0;

            const size_t accessBlueRegion = x - glitchOffset;
            (accessBlueRegion < image->width)
                    ? (pixel->blue = rowCopy[accessBlueRegion].blue)
                    : 0;
        }
    }

    // Free temp memory and exit
    free(rowCopy);
    return EXIT_SUCCESS;
}

[[nodiscard]] int verify_offset_bounds(Image* image, const size_t offset) // FIX
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

    FX_TEMPLATE(image, { // Apply contrast filter to each colour
        pixel->blue = lookupTable[pixel->blue];
        pixel->green = lookupTable[pixel->green];
        pixel->red = lookupTable[pixel->red];
    });
}

void dim_effect(Image* image, const uint8_t redDim, const uint8_t greenDim,
        const uint8_t blueDim)
{
    FX_TEMPLATE(image,
            { // Reduce value of each pixel component by the dimming factor
              // Zero is the minimum value components can be reduced to.
                pixel->blue = clamp_floor_u8(pixel->blue, blueDim);
                pixel->green = clamp_floor_u8(pixel->green, greenDim);
                pixel->red = clamp_floor_u8(pixel->red, redDim);
            });
}

uint8_t contrast_effect_val(const uint8_t val, const uint8_t contrastFactor,
        const uint8_t min, const uint8_t max)
{
    if (val >= max) {
        return clamp_ceil_u8(val, contrastFactor);
    }

    if (val <= min) {
        return clamp_floor_u8(val, contrastFactor);
    }

    return val;
}

void swap_red_blue(Image* image)
{
    FX_TEMPLATE(image, { // Swap red and blue values
        const uint8_t temp = pixel->red;
        pixel->red = pixel->blue;
        pixel->blue = temp;
    });
}

int cmp_pixels(const void* a, const void* b)
{
    const Pixel* p1 = (const Pixel*)a;
    const Pixel* p2 = (const Pixel*)b;

    int s1 = p1->blue + p1->green + p1->red;
    int s2 = p2->blue + p2->green + p2->red;

    return (s1 - s2);
}

[[nodiscard]] int melt(BMP* bmp, const int32_t startPoint)
{
    Image* image = bmp->image;
    bool inv = false;

    size_t norm = 0;
    if (startPoint == 0) {
        return -1; // FIX
    }

    if (startPoint < 0) {
        norm = (size_t)(-startPoint);
        inv = true;
    } else {
        norm = (size_t)startPoint;
    }
    norm--;

    if (norm > image->height) {
        norm = 1;
    }

    Image* rotated = NULL;

    if (inv) {
        rotated = rotate_image_clockwise(image);
    } else {
        rotated = rotate_image_anticlockwise(image);
    }
    if (rotated == NULL) {
        return -1;
    }

    const size_t nmembPix = rotated->width - norm;

    for (size_t y = 0; y < rotated->height; y++) {
        size_t rowOffset = y * rotated->width;
        Pixel* rowPtr = get_pixel_fast(rotated, norm, rowOffset);

        qsort(rowPtr, nmembPix, sizeof(Pixel), cmp_pixels);
    }

    Image* unRotated = NULL;

    if (inv) {
        unRotated = rotate_image_anticlockwise(rotated);
    } else {
        unRotated = rotate_image_clockwise(rotated);
    }

    free_image(&rotated);

    if (unRotated == NULL) {
        return -1;
    }

    free_image(&image);
    bmp->image = unRotated;
    return EXIT_SUCCESS;
}

void colour_scaler(
        Image* image, const double red, const double green, const double blue)
{
    FX_TEMPLATE(image, { // Swap red and blue values
        pixel->blue = (uint8_t)(pixel->blue * blue);
        pixel->green = (uint8_t)(pixel->green * green);
        pixel->red = (uint8_t)(pixel->red * red);
    });
}

Image* image_blur(Image* image, uint32_t radius)
{
    size_t perimeter = (radius << 1) + 1;
    size_t volume = perimeter * perimeter;

    Pixel** buffer = malloc(volume * sizeof(Pixel*));
    size_t* rowBuff = malloc(perimeter * sizeof(size_t));

    Image* new = create_image((int32_t)image->width, (int32_t)image->height);

    for (size_t y = radius; y < image->height - radius; y++) {
        size_t ggVal = y - radius;

        for (size_t j = 0; j < perimeter; j++) {
            rowBuff[j] = image->width * (ggVal + j);
        }

        for (size_t x = radius; x < image->width - radius; x++) {
            const size_t val = x - radius;

            for (size_t w = 0; w < perimeter; w++) {

                for (size_t z = 0; z < perimeter; z++) {
                    const size_t newVal = val + z;

                    buffer[perimeter * w + z]
                            = get_pixel_fast(image, newVal, rowBuff[w]);
                }
            }

            size_t bSum = 0;
            size_t gSum = 0;
            size_t rSum = 0;

            for (size_t i = 0; i < volume; i++) {
                bSum += (buffer[i])->blue;
                gSum += (buffer[i])->green;
                rSum += (buffer[i])->red;
            }

            Pixel* pPixel = get_pixel(new, x, y);
            pPixel->blue = (uint8_t)(bSum / volume);
            pPixel->green = (uint8_t)(gSum / volume);
            pPixel->red = (uint8_t)(rSum / volume);
        }
    }

    // Free temporary resources
    free(buffer);
    free(rowBuff);

    return new;
}
