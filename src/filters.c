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

// Constants to best map RGB values to grayscale
// Each multiplied by 1024 to avoid floats
constexpr int rMapGS = 306; // 0.299
constexpr int gMapGS = 601; // 0.587
constexpr int bMapGS = 117; // 0.114

// Used to optimise division by 3 (for positive integers less than 765)
constexpr int div3Const = 683;
constexpr int div3Shift = 11;

// Used for division by 1024 (more efficient than dividing by 1000)
constexpr int pixScaleMultGS = 10;

/* calc_pixel_average()
 * --------------------
 * Calculate the average intensity of an input Pixels colour channels.
 *
 * pixel: input Pixel.
 *
 * Returns: The average intensity.
 */
static inline uint8_t calc_pixel_average(Pixel* pixel)
{
    return (uint8_t)(((pixel->red + pixel->green + pixel->blue) * div3Const)
            >> div3Shift);
}

/* calc_pixel_grayscale()
 * ----------------------
 * Uses the Luma RGB intensity coefficients to calculate the intensity value
 * required to make the Pixel appear grayscaled.
 *
 * pixel: input Pixel.
 *
 * Returns: The grayscale value.
 */
static inline uint8_t calc_pixel_grayscale(Pixel* pixel)
{
    // Calculate gray scaled value
    const uint32_t temp = (uint32_t)(rMapGS * pixel->red + gMapGS * pixel->green
            + bMapGS * pixel->blue);

    return (uint8_t)(temp >> pixScaleMultGS);
}

/* clamp_floor_u8()
 * ----------------
 * Attempts to perform a subtraction of two uint8_t integers, if the subtraction
 * would result in underflow, the result is clamped from below and set to 0.
 *
 * val: Main value.
 * sub: Value subtracted from val.
 *
 * Returns: The uint8_t result the clamped subtraction.
 */
static inline uint8_t clamp_floor_u8(const uint8_t val, const uint8_t sub)
{
    return (uint8_t)((val > sub) ? (val - sub) : (0));
}

/* clamp_ceil_u8()
 * ---------------
 * Attempts to perform an addition of two uint8_t integers, if the addition
 * would result in overflow, the result is clamped above and set to UINT8_MAX.
 *
 * val: Main value.
 * add: Value added to val.
 *
 * Returns: The uint8_t result after the clamped addition.
 */
static inline uint8_t clamp_ceil_u8(const uint8_t val, const uint8_t add)
{
    const int sum = val + add;
    return (uint8_t)((sum >= UINT8_MAX) ? (UINT8_MAX) : (sum));
}

/* ave_u8_2x()
 * -----------
 * Calculates the arithmetic mean of two uint8_t integers.
 *
 * a: Number 1.
 * b: Number 2.
 *
 * Returns: The arithmetic mean.
 */
static inline uint8_t ave_u8_2x(const uint8_t a, const uint8_t b)
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

int combine_images(Image* restrict primary, const Image* restrict secondary)
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
        const size_t rowOffset = y * width;

        Pixel* pRowPtr = get_pixel_fast(primary, 0, rowOffset);
        Pixel* sRowPtr = get_pixel_fast(secondary, 0, rowOffset);

        // For each pixel in row
        _Pragma("omp simd") for (size_t x = 0; x < width; x++)
        {
            // For reduced cpu cycles
            Pixel* pPixel = pRowPtr + x;
            Pixel* sPixel = sRowPtr + x;

            // Average the each colour value from each image and update
            // value in primary image.
            pPixel->blue = ave_u8_2x(pPixel->blue, sPixel->blue);
            pPixel->green = ave_u8_2x(pPixel->green, sPixel->green);
            pPixel->red = ave_u8_2x(pPixel->red, sPixel->red);
        }
    }

    return EXIT_SUCCESS;
}

int merge_images(Image* restrict primary, const Image* restrict secondary)
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
        const size_t rowOffset = y * width;

        Pixel* pRowPtr = get_pixel_fast(primary, 0, rowOffset);
        Pixel* sRowPtr = get_pixel_fast(secondary, 0, rowOffset);

        // For each pixel in row
        _Pragma("omp simd") for (size_t x = 0; x < width; x++)
        {

            // For reduced cpu cycles
            Pixel* pPixel = pRowPtr + x;
            Pixel* sPixel = sRowPtr + x;

            // Average the each colour value from each image and update
            // value in primary image.
            pPixel->blue = clamp_ceil_u8(pPixel->blue, sPixel->blue);
            pPixel->green = clamp_ceil_u8(pPixel->green, sPixel->green);
            pPixel->red = clamp_ceil_u8(pPixel->red, sPixel->red);
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

/* contrast_effect_val()
 * ---------------------
 * Helper function to calculate the resulting pixel intensity provided contrast
 * effect parameters.
 *
 * val: Current intensity of a pixel component.
 * contrastFactor: Level of contrasting.
 * min: Lower bound of colour intensity to be effected.
 * max: Upper bound of colour intensity to be effected.
 *
 * Returns: Colour intensity for a pixel component.
 */
static inline uint8_t contrast_effect_val(const uint8_t val,
        const uint8_t contrastFactor, const uint8_t min, const uint8_t max)
{
    if (val >= max) {
        return clamp_ceil_u8(val, contrastFactor);
    }

    if (val <= min) {
        return clamp_floor_u8(val, contrastFactor);
    }

    return val;
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

void apply_hue(Image* image, const int red, const int green, const int blue)
{
    int sum;
    FX_TEMPLATE(image, {
        sum = (int)(pixel->blue) + blue;
        if (sum < 0) {
            pixel->blue = (uint8_t)(0);
        } else if (sum >= UINT8_MAX) {
            pixel->blue = (uint8_t)(UINT8_MAX);
        } else {
            pixel->blue = (uint8_t)(sum);
        }

        sum = (int)(pixel->green) + green;
        if (sum < 0) {
            pixel->green = (uint8_t)(0);
        } else if (sum >= UINT8_MAX) {
            pixel->green = (uint8_t)(UINT8_MAX);
        } else {
            pixel->green = (uint8_t)(sum);
        }

        sum = (int)(pixel->red) + red;
        if (sum < 0) {
            pixel->red = (uint8_t)(0);
        } else if (sum >= UINT8_MAX) {
            pixel->red = (uint8_t)(UINT8_MAX);
        } else {
            pixel->red = (uint8_t)(sum);
        }
    });
}

[[deprecated]] void dim_effect(Image* image, const uint8_t redDim,
        const uint8_t greenDim, const uint8_t blueDim)
{
    FX_TEMPLATE(image,
            { // Reduce value of each pixel component by the dimming factor
              // Zero is the minimum value components can be reduced to.
                pixel->blue = clamp_floor_u8(pixel->blue, blueDim);
                pixel->green = clamp_floor_u8(pixel->green, greenDim);
                pixel->red = clamp_floor_u8(pixel->red, redDim);
            });
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

[[nodiscard]] int melt(BMP* bmp, const int32_t start)
{
    Image* image = bmp->image;
    bool inv = false;

    size_t norm = 0;
    if (start == 0) {
        return -1; // FIX
    }

    if (start < 0) {
        norm = (size_t)(-start);
        inv = true;
    } else {
        norm = (size_t)start;
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

/* bound_double_to_u8()
 * --------------------
 * Clamps a double precision floating point value above/below by UINT8_MAX and 0
 * respectively. This is to prevent overflow during type casting to uint8_t.
 *
 * val: The input floating point value.
 *
 * Returns: The clamped uint8_t value.
 */
static inline uint8_t bound_double_to_u8(const double val)
{
    double d = val;

    d = (d > 255.0) ? (255.0) : (d);
    d = (d > 0.0) ? (d) : (0.0);

    return (uint8_t)d;
}

void colour_scaler_strict(
        Image* image, const double red, const double green, const double blue)
{
    FX_TEMPLATE(image, {
        pixel->blue = bound_double_to_u8(pixel->blue * blue);
        pixel->green = bound_double_to_u8(pixel->green * green);
        pixel->red = bound_double_to_u8(pixel->red * red);
    });
}

void colour_scaler(
        Image* image, const double red, const double green, const double blue)
{
    FX_TEMPLATE(image, {
        pixel->blue = (uint8_t)(pixel->blue * blue);
        pixel->green = (uint8_t)(pixel->green * green);
        pixel->red = (uint8_t)(pixel->red * red);
    });
}

/* blurred_pixel_row()
 * -------------------
 * Applies a horizontal box blur to a single row of an image using a sliding
 * window.
 *
 * image: Destination image to store the blurred row.
 * buffer: Source array containing the original pixel row data.
 * rNumber: The vertical index of the row being processed.
 * radius: The radius of the blur.
 * perimeter: The total width of the blur effect (radius * 2 + 1).
 */
static inline void blurred_pixel_row(Image* image, const Pixel* buffer,
        const size_t rNumber, const size_t radius, const size_t perimeter)
{
    const size_t rOffset = rNumber * image->width;
    size_t blueSum = 0;
    size_t greenSum = 0;
    size_t redSum = 0;

    // Initial average generation (equivilent to -1 index)
    for (size_t i = 0; i < radius; i++) {
        blueSum += (size_t)((buffer[i]).blue);
        greenSum += (size_t)((buffer[i]).green);
        redSum += (size_t)((buffer[i]).red);
    }

    for (size_t x = 0; x < image->width; x++) {
        Pixel* p = get_pixel_fast(image, x, rOffset);

        if (x <= radius) { // Just adding
            Pixel last = buffer[x + radius];
            blueSum += (size_t)(last.blue);
            greenSum += (size_t)(last.green);
            redSum += (size_t)(last.red);

            const size_t nmemb = x + 1 + radius;
            p->blue = (uint8_t)(blueSum / nmemb);
            p->green = (uint8_t)(greenSum / nmemb);
            p->red = (uint8_t)(redSum / nmemb);

        } else if (x + radius + 1 > image->width) { // Just subtracting
            Pixel first = buffer[x - radius - 1];
            blueSum -= (size_t)(first.blue);
            greenSum -= (size_t)(first.green);
            redSum -= (size_t)(first.red);

            const size_t nmemb = image->width - x + radius;
            p->blue = (uint8_t)(blueSum / nmemb);
            p->green = (uint8_t)(greenSum / nmemb);
            p->red = (uint8_t)(redSum / nmemb);

        } else { // Adding and subtracting
            Pixel first = buffer[x - radius - 1];
            Pixel last = buffer[x + radius];

            blueSum += (size_t)(last.blue) - (size_t)(first.blue);
            greenSum += (size_t)(last.green) - (size_t)(first.green);
            redSum += (size_t)(last.red) - (size_t)(first.red);

            p->blue = (uint8_t)(blueSum / perimeter);
            p->green = (uint8_t)(greenSum / perimeter);
            p->red = (uint8_t)(redSum / perimeter);
        }
    }
}

// O(1)
Image* even_faster_image_blur(const Image* restrict image, const size_t radius)
{
    Image* t1 = create_image((int32_t)image->width, (int32_t)image->height);
    if (t1 == NULL) {
        return NULL;
    }

    const size_t nPixelsMax
            = (image->width > image->height) ? (image->width) : (image->height);

    Pixel* buffer = malloc(nPixelsMax * sizeof(Pixel));
    if (buffer == NULL) {
        free_image(&t1);
        return NULL;
    }

    const size_t perimeter = (radius << 1) + 1;
    const size_t rSizeT1 = image->width * sizeof(Pixel);

    for (size_t y = 0; y < image->height; y++) {
        Pixel* p = get_pixel_fast(image, 0, y * image->width);
        memcpy(buffer, p, rSizeT1);
        blurred_pixel_row(t1, buffer, y, radius, perimeter);
    }

    Image* t2 = transpose_image(t1);
    free_image(&t1);

    if (t2 == NULL) {
        free(buffer);
        return NULL;
    }

    const size_t rSizeT2 = t2->width * sizeof(Pixel);

    for (size_t y = 0; y < t2->height; y++) {
        Pixel* p = get_pixel_fast(t2, 0, y * t2->width);
        memcpy(buffer, p, rSizeT2);
        blurred_pixel_row(t2, buffer, y, radius, perimeter);
    }

    Image* final = transpose_image(t2);
    free(buffer);
    free_image(&t2);
    return final;
}

// O(R)
[[deprecated]] Image* faster_image_blur(
        const Image* restrict image, const size_t radius)
{
    Image* new = create_image((int32_t)image->width, (int32_t)image->height);
    size_t perimeter = (radius << 1) + 1;

    Pixel* buffer = malloc(sizeof(Pixel) * image->width);

    if (buffer == NULL) {
        free_image(&new);
        return NULL;
    }

    size_t* lookup = malloc(perimeter * sizeof(size_t));
    if (lookup == NULL) {
        free_image(&new);
        free(buffer);
        return NULL;
    }

    for (size_t row = radius; row < image->height - radius; row++) {

        // Populate row pointer
        size_t start = row - radius;
        for (size_t i = 0; i < perimeter; i++) {
            lookup[i] = image->width * (start++);
        }

        for (size_t x = 0; x < image->width; x++) {

            size_t bAve = 0;
            size_t gAve = 0;
            size_t rAve = 0;

            for (size_t y = 0; y < perimeter; y++) {
                Pixel* pix = get_pixel_fast(image, x, lookup[y]);
                bAve += (size_t)(pix->blue);
                gAve += (size_t)(pix->green);
                rAve += (size_t)(pix->red);
            }

            Pixel average = {
                    .blue = (uint8_t)(bAve / perimeter),
                    .green = (uint8_t)(gAve / perimeter),
                    .red = (uint8_t)(rAve / perimeter),
            };

            buffer[x] = average;
        }

        blurred_pixel_row(new, buffer, row, radius, perimeter);
    }

    free(buffer);
    free(lookup);
    return new;
}

void edge_detection(Image* image, const int threshold)
{
    for (size_t y = 0; y < image->height; y++) {
        size_t rowOffset = y * image->width;
        Pixel* rowPtr = get_pixel_fast(image, 0, rowOffset);

        for (size_t x = 0; x < image->width - 1; x++) {

            Pixel* cPixel = rowPtr + x;
            Pixel* nPixel = rowPtr + x + 1;

            int diffSum = 0;
            diffSum += (int)(nPixel->blue - cPixel->blue);
            diffSum += (int)(nPixel->green - cPixel->green);
            diffSum += (int)(nPixel->red - cPixel->red);

            if (diffSum > threshold) {
                nPixel->blue = UINT8_MAX;
                nPixel->green = 0;
                nPixel->red = 0;
            } else if (-diffSum > threshold) {
                cPixel->blue = UINT8_MAX;
                cPixel->green = 0;
                cPixel->red = 0;
            }
        }
    }
}
