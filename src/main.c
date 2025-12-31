// Included Libraries
#include "fileParsing.h"
#include "filters.h"
#include "imageEditing.h"
#include "main.h"
#include "utils.h"
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <limits.h>

// Program constant strings
constexpr char usageMessage[] = "Usage: ./signals <option> [--input <file>] ...\n";
constexpr char helpMessage[] // Need to update FIX
        = "Usage: ./signals <option> [--input <file>] ...\n"
          "\n"
          "Help:\n"
          "  -h, --help                  - Show this help message\n"
          "\n"
          "File I/O Options:\n"
          "  -i, --input <file>          - Input BMP file to process\n"
          "  -o, --output <file>         - Output file path to write result\n"
          "  -c, --combine <file>        - Overlay another BMP image onto "
          "input\n"
          "  -m, --merge <file>          - Average blend with another BMP "
          "image\n"
          "  -d, --dump                  - Dump BMP header information to "
          "terminal\n"
          "  -p, --print                 - Render image to terminal (ANSI)\n"
          "  -e, --encode <file>         - Reads contents of <file>, and "
          "embeds into image\n"
          "\n"
          "Colours & Channels:\n"
          "  -f, --filter <channels>     - Isolate specific channels (e.g. "
          "'rb')\n"
          "  -g, --grayscale             - Convert image to grayscale (Luma)\n"
          "  -a, --average               - Convert to grayscale (Average "
          "Intensity)\n"
          "  -v, --invert                - Invert image colours\n"
          "  -s, --swap                  - Swap Red and Blue color channels\n"
          "Geometry & Orientation:\n"
          "  -r, --rotate <90s>          - Rotate image 90 degrees clockwise N "
          "  -R, --reverse               - Reverse image horizontally\n"
          "  -F, --flip                  - Flip image vertically\n"
          "times\n"
          "\n"

          "Brightness & Contrast:\n"
          "  -C, --contrast <val>        - Adjust contrast factor (0-255)\n"
          "  -b, --brightness-cut <val>  - Cuts pixel component if value "
          "exceeds cutoff (0-255)\n"
          "  -D, --dim <val>             - Reduce brightness by value (0-255)\n"
          "  -T, --scale-strict <val>    - Scale colour intensity "
          "(multiplier)\n"
          "\n"

          "Advanced Effects:\n"
          "  -M, --melt <offset>         - Pixel sorting effect (negative to "
          "invert)\n"
          "  -G, --glitch <offset>       - Apply horizontal glitch effect\n"
          "  -B, --blur <radius>         - Blurs the image using the set "
          "radius\n"
          "  -S, --scale <val>           - Scale colour intensity (overflow "
          "allowed)\n"
          "  -E, --experimental          - Try out an experimental feature!\n";

constexpr char fileTypeMessage[] = "Input file must be \".bmp\"\n";
constexpr char emptyArgsMessage[] = "Arguments must not be empty.\n";
constexpr char noArgsProvidedMessage[] = "An argument must be supplied.\n";
constexpr char userHelpPrompt[]
        = "Enter: \"./signals --help\" for available commands\n";
constexpr char nonUniquePathsMessage[]
        = "Input and combine file paths must be unique!\n";
constexpr char fileType[] = ".bmp";

// Error messages
constexpr char unexpectedArgMessage[] = "Got \"%s\", expected \"%s\"\n";
constexpr char gotStrMessage[] = "    Got \"%s\".\n";
constexpr char invalidFilterColourMessage[]
        = "Filter colour/s invalid \"%s\", must be RGB characters.\n";

// Assorted constant chars
const char* const readMode = "rb";
const char* const writeMode = "wb";

typedef enum {
    INVALID = -1,

    // Help:
    HELP = 'h',

    // I/O:
    INPUT_FILE = 'i',
    OUTPUT_FILE = 'o',
    MERGE = 'm',
    COMBINE = 'c',
    DUMP_HEADER = 'd',
    PRINT_IMAGE = 'p',
    ENCODE = 'e',

    // Colours & Channels:
    FILTERS = 'f',
    GRAY_SCALE = 'g',
    AVE = 'a',
    INVERT = 'v',
    SWAP = 's',

    // Geometry & Orientation:
    ROTATE = 'r',
    REVERSE = 'R',
    FLIP = 'F',

    // Brightness & Contrast:
    CONTRAST = 'C',
    BRIGHTNESS_CUT = 'b',
    DIM = 'D',
    SCALE_STRICT = 'T',

    // Advanced Effects:
    MELT = 'M',
    GLITCH = 'G',
    SCALE = 'S',
    BLUR = 'B',

    EXPERIMENTAL = 'E',
} Flag;

constexpr char optstring[]
        = "i:o:m:c:e:f:r:C:b:D:T:M:G:S:B:hdpgavsRFE"; // Defined program flags

static struct option const longOptions[] = {
        {"input", required_argument, NULL, INPUT_FILE},
        {"output", required_argument, NULL, OUTPUT_FILE},
        {"dump", no_argument, NULL, DUMP_HEADER},
        {"print", no_argument, NULL, PRINT_IMAGE},
        {"help", no_argument, NULL, HELP},
        {"filter", required_argument, NULL, FILTERS},
        {"grayscale", no_argument, NULL, GRAY_SCALE},
        {"invert", no_argument, NULL, INVERT},
        {"flip", no_argument, NULL, FLIP},
        {"brightness-cut", required_argument, NULL, BRIGHTNESS_CUT},
        {"combine", required_argument, NULL, COMBINE},
        {"glitch", required_argument, NULL, GLITCH},
        {"average", no_argument, NULL, AVE},
        {"contrast", required_argument, NULL, CONTRAST},
        {"dim", required_argument, NULL, DIM},
        {"swap", no_argument, NULL, SWAP},
        {"rotate", required_argument, NULL, ROTATE},
        {"reverse", no_argument, NULL, REVERSE},
        {"melt", required_argument, NULL, MELT},
        {"scale", required_argument, NULL, SCALE},
        {"scale-strict", required_argument, NULL, SCALE_STRICT},
        {"merge", required_argument, NULL, MERGE},
        {"blur", required_argument, NULL, BLUR},
        {"encode", required_argument, NULL, ENCODE},
        {"experimental", no_argument, NULL, EXPERIMENTAL},
        {NULL, 0, NULL, 0},
};

int main(const int argc, char* argv[])
{
    if (early_argument_checks(argc, argv) != EXIT_SUCCESS) {
        exit(EXIT_INVALID_ARG);
    }

    // Initialise
    UserInput userInput;
    (void)memset(&userInput, 0, sizeof(userInput));

    int status;
    if ((status = parse_user_commands(argc, argv, &userInput))
            != EXIT_SUCCESS) {
        exit(status);
    }

    exit(handle_commands(&userInput));
}

int check_for_empty_args(const int argc, char** argv)
{
    for (int i = 0; i < argc; i++) {
        if (argv[i][0] == eos) {
            fputs(emptyArgsMessage, stderr);
            fputs(usageMessage, stderr);
            return -1;
        }
    }

    return EXIT_SUCCESS;
}

int early_argument_checks(const int argc, char** argv)
{
    // Check if user did not supply any arguments
    if (!(argc >= MIN_CMD_ARGS)) {

        // Print error messages
        fputs(noArgsProvidedMessage, stderr);
        fputs(usageMessage, stderr);
        fputs(userHelpPrompt, stderr);
        return -1;
    }

    if (check_for_empty_args(argc, argv) == -1) {
        return -1;
    }

    // Checks passed
    return EXIT_SUCCESS;
}

int parse_user_commands(const int argc, char** argv, UserInput* userInput)
{
    Flag opt;

    // Loop over all of the options
    while ((opt = getopt_long(argc, argv, optstring, longOptions, NULL))
            != -1) {
        switch (opt) {

        case HELP:
            userInput->help = true;
            break;

        case DUMP_HEADER:
            userInput->header = true;
            break;

        case PRINT_IMAGE:
            userInput->print = true;
            break;

        case INPUT_FILE:
            userInput->input = true;
            userInput->inputFilePath = optarg;
            break;

        case OUTPUT_FILE:
            userInput->output = true;
            userInput->outputFilePath = optarg;
            break;

        case FILTERS: {
            if (filtr_col_check(&(userInput->filters), optarg) == -1) {
                return EXIT_INVALID_PARAMETER;
            }
            break;
        }

        case GRAY_SCALE:
            userInput->grayscale = true;
            break;

        case INVERT:
            userInput->invert = true;
            break;

        case FLIP:
            userInput->flip = true;
            break;

        case BRIGHTNESS_CUT: { // If brightness cut flag used, verify the
                               // required argument
            if (!(vlongB(
                        &(userInput->cutoff), optarg, 0, UINT8_MAX, uint8_t))) {
                return EXIT_INVALID_PARAMETER;
            }
            break;
        }

        case COMBINE:
            userInput->combine = true;
            userInput->combineFilePath = optarg;
            break;

        case MERGE:
            userInput->merge = true;
            userInput->mergeFilePath = optarg;
            break;

        case ENCODE:
            userInput->encode = true;
            userInput->encodeFilePath = optarg;
            break;

        case GLITCH: { // If glitch flag used, verify the glitch offset
            if (!(vlongB(&(userInput->glitch), optarg, 1, INT32_MAX, size_t))) {
                glitch_offset_invalid_message(optarg); // Prints error messages
                return EXIT_INVALID_PARAMETER;
            }
            break;
        }

        case AVE:
            userInput->average = true;
            break;

        case CONTRAST: { // If contrast flag set, verify the required argument
            if (!(vlongB(&(userInput->contrast), optarg, 0, UINT8_MAX,
                        uint8_t))) {
                return EXIT_INVALID_PARAMETER;
            }
            break;
        }

        case DIM: { // If dim flag set, verify the required argument
            if (!(vlongB(&(userInput->dim), optarg, 0, UINT8_MAX, uint8_t))) {
                return EXIT_INVALID_PARAMETER;
            }
            break;
        }

        case SWAP:
            userInput->swap = true;
            break;

        case ROTATE: {
            if (!(vlongB(&(userInput->rotations), optarg, LONG_MIN, LONG_MAX,
                        long))) {
                return EXIT_INVALID_PARAMETER;
            }
            break;
        }

        case REVERSE:
            userInput->reverse = true;
            break;

        case MELT: {
            if (!verify_melt(userInput, optarg)) {
                return EXIT_INVALID_PARAMETER;
            }
            break;
        }

        case SCALE:
            userInput->scale = atof(optarg);
            break;

        case SCALE_STRICT: {
            userInput->scaleStrict = atof(optarg);
            break;
        }

        case BLUR: {
            if (!(vlongB(
                        &(userInput->blur), optarg, 1, UINT32_MAX, uint32_t))) {
                return EXIT_INVALID_PARAMETER;
            }
            break;
        }

        case EXPERIMENTAL:
            userInput->experimental = true;
            break;

            // If valid command supplied, or none supplied at all
        default:
            return EXIT_NO_COMMAND;
        }
    }

    // All options parses successfully
    return EXIT_SUCCESS;
}

bool verify_melt(UserInput* userInput, const char* arg)
{
    if (!(vlongB(&(userInput->meltOffset), arg, INT32_MIN, INT32_MAX,
                int32_t))) {
        return false;
    }

    // Could remove since this would have no effect if activated
    if (userInput->meltOffset == 0) {
        return false;
    }

    return true;
}

void glitch_offset_invalid_message(const char* arg)
{
    fputs(glitchUsageMessage, stderr);
    fputs(glitchOffsetValMessage, stderr);
    fprintf(stderr, gotStrMessage, arg);
}

int handle_commands(UserInput* userInput)
{
    if (userInput->help) { // If help mode enabled
        fputs(helpMessage, stdout);
    }

    // An input file is required all non-help commands
    if (!(userInput->input)) {
        return EXIT_MISSING_INPUT_FILE;
    }

    // Check the file type of supplied file path
    if (check_valid_file_type(fileType, userInput->inputFilePath) == -1) {
        return EXIT_INVALID_FILE_TYPE;
    }

    // Initialise struct to store BMP data
    BMP bmpImage;
    initialise_bmp(&bmpImage);
    int status = EXIT_SUCCESS;

    // Goes through command modes, ensuring all memory is freed, and correct
    // exit status is recorded for return value.
    while (1) {
        // Attempt to open the BMP and read file headers
        if ((status = open_bmp(&bmpImage, userInput->inputFilePath))
                != EXIT_SUCCESS) {
            break;
        }

        if (userInput->header) { // If header dump mode enabled
            dump_headers(&bmpImage);
        }

        // Attempt to load pixel data from file into bmpImage struct
        if ((status = handle_bmp_loading(&bmpImage)) != EXIT_SUCCESS) {
            break;
        }

        if (userInput->experimental) {
            edge_detection(bmpImage.image, 400);
            Image* rotated = rotate_image_clockwise(bmpImage.image);
            edge_detection(rotated, 400);
            Image* unrotated = rotate_image_anticlockwise(rotated);
            free_image(&rotated);
            free_image(&(bmpImage.image));
            bmpImage.image = unrotated;
        }

        if (userInput->flip) { // If flip mode enabled
            if (flip_image(bmpImage.image) == -1) {
                break;
            }
        }

        if (userInput->filters) {
            ColourFilter filter
                    = (ColourFilter)handle_colour_filters(userInput->filters);
            if (filter == NULL) { // FIX check this is even possible
                break; // FIX update status add add error message
            }

            filter(bmpImage.image);
        }

        if (userInput->scale) {
            colour_scaler(bmpImage.image, userInput->scale, userInput->scale,
                    userInput->scale);
        }

        if (userInput->scaleStrict) {
            colour_scaler_strict(bmpImage.image, userInput->scaleStrict,
                    userInput->scaleStrict, userInput->scaleStrict);
        }

        if (userInput->cutoff) {
            brightness_cut_filter(bmpImage.image, userInput->cutoff);
        }

        if (userInput->grayscale) { // If grayscale mode enabled
            gray_filter(bmpImage.image);
        }

        if (userInput->glitch) { // If glitch mode enabled
            if (glitch_effect(bmpImage.image, userInput->glitch) == -1) {
                status = EXIT_OUT_OF_BOUNDS; // FIX
                break;
            }
        }

        if (userInput->swap) { // If swap mode enabled
            swap_red_blue(bmpImage.image);
        }

        if (userInput->contrast) { // If contrast mode enabled
            contrast_effect( // Prev 100 160
                    bmpImage.image, userInput->contrast, 80, 200); // FIX Magic
        }

        if (userInput->blur) {
            // FIX add error
            Image* blurred = image_blur(bmpImage.image, userInput->blur);
            free_image(&(bmpImage.image));
            bmpImage.image = blurred;
        }

        if (userInput->dim) { // If dim mode enabled
            dim_effect(bmpImage.image, userInput->dim, userInput->dim,
                    userInput->dim); // FIX Update docs
        }

        if (userInput->melt) {
            if (melt(&bmpImage, userInput->meltOffset) == -1) {
                puts("Melt failed"); // FIX
                break;
            }
        }

        if (userInput->combine) { // If image combination mode enabled
            if ((status = handle_combine(userInput, &bmpImage))
                    != EXIT_SUCCESS) {
                fprintf(stderr, "Combine failed\n");
                break;
            }
        }

        if (userInput->merge) { // If image combination mode enabled
            if ((status = handle_merge(userInput, &bmpImage)) != EXIT_SUCCESS) {
                fprintf(stderr, "Merge failed\n");
                break;
            }
        }

        if (userInput->invert) { // If image inversion mode enabled
            invert_colours(bmpImage.image);
        }

        if (userInput->average) { // If pixel averaging mode enabled
            average_pixels(bmpImage.image);
        }

        if (userInput->rotations) {
            handle_image_rotation(&bmpImage, userInput->rotations);
            if (bmpImage.image == NULL) {
                break; // FIX add specific error code
            }

            if (userInput->rotations % 2) {
                const int32_t temp = bmpImage.infoHeader.bitmapWidth;
                bmpImage.infoHeader.bitmapWidth
                        = bmpImage.infoHeader.bitmapHeight;
                bmpImage.infoHeader.bitmapHeight = temp;
            }
        }

        if (userInput->reverse) {
            reverse_image(bmpImage.image);
        }

        if (userInput->output) { // If output file mode enabled
            if (userInput->encode) {
                if (write_bmp_with_header_provided(&bmpImage,
                            userInput->outputFilePath,
                            userInput->encodeFilePath)
                        == -1) {
                    status = EXIT_OUTPUT_FILE_ERROR;
                    break;
                }
            } else {

                if (write_bmp_with_header_provided(
                            &bmpImage, userInput->outputFilePath, NULL)
                        == -1) {
                    status = EXIT_OUTPUT_FILE_ERROR;
                    break;
                }
            }
        }

        if (userInput->print) { // If terminal printing mode enabled
            if (flip_image(bmpImage.image) == -1) {
                break;
            }

            print_image_to_terminal(bmpImage.image);
        }

        break;
    }

    // Cleanup and exit
    free_image_resources(&bmpImage);
    return status;
}

Function handle_colour_filters(const uint8_t filters)
{
    const uint8_t index = filters & 0x07;

    const Function filterMap[8] = {
            NULL,
            (Function)&filter_red,
            (Function)&filter_green,
            (Function)&filter_red_green,
            (Function)&filter_blue,
            (Function)&filter_red_blue,
            (Function)&filter_green_blue,
            (Function)&filter_all,
    };

    return filterMap[index];
}

int filtr_col_check(uint8_t* setting, const char* arg)
{
    const char allowed[] = {'r', 'R', 'g', 'G', 'b', 'B', eos};
    uint8_t colourBitVect = 0;

    for (size_t i = 0; i < strlen(arg); i++) {

        size_t j = 0;
        while (1) {

            // Character did not match any valid char.
            if (allowed[j] == eos) {
                fprintf(stderr, invalidFilterColourMessage, arg);
                *setting = 0;
                return -1;
            }

            if ((char)(arg[i]) == allowed[j]) {

                // Sets bit in bit vector to indicate presence of colour to
                // filter. Both 'r' and 'R' are represented by the same bit,
                // hence the division by two.
                colourBitVect |= (uint8_t)(1 << (j / 2)); // FIX optimise
                break;
            }
            j++;
        }
    }

    *setting = colourBitVect;
    return EXIT_SUCCESS;
}

int handle_combine(const UserInput* userInput, BMP* bmpImage)
{
    if (check_valid_file_type(fileType, userInput->combineFilePath) == -1) {
        return EXIT_INVALID_FILE_TYPE;
    }

    // Exit if input and combine file paths match
    if (!strcmp(userInput->inputFilePath, userInput->combineFilePath)) {
        fputs(nonUniquePathsMessage, stderr);
        return EXIT_SAME_FILE;
    }

    BMP combinedImage;
    initialise_bmp(&combinedImage);
    int status = EXIT_SUCCESS;

    while (1) {
        if ((status = open_bmp(&combinedImage, userInput->combineFilePath))
                != EXIT_SUCCESS) {
            break;
        }

        if ((status = handle_bmp_loading(&combinedImage)) != EXIT_SUCCESS) {
            break;
        }

        if ((status = combine_images(bmpImage->image, combinedImage.image))
                != EXIT_SUCCESS) {
            break;
        }

        break;
    }

    // Cleanup and exit
    free_image_resources(&combinedImage);
    return status;
}

int handle_merge(const UserInput* userInput, BMP* bmpImage)
{
    if (check_valid_file_type(fileType, userInput->mergeFilePath) == -1) {
        return EXIT_INVALID_FILE_TYPE;
    }

    // Exit if input and merge file paths match
    if (!strcmp(userInput->inputFilePath, userInput->mergeFilePath)) {
        fputs(nonUniquePathsMessage, stderr);
        return EXIT_SAME_FILE;
    }

    BMP mergedImage;
    initialise_bmp(&mergedImage);
    int status = EXIT_SUCCESS;

    while (1) {
        if ((status = open_bmp(&mergedImage, userInput->mergeFilePath))
                != EXIT_SUCCESS) {
            break;
        }

        if ((status = handle_bmp_loading(&mergedImage)) != EXIT_SUCCESS) {
            break;
        }

        if ((status = merge_images(bmpImage->image, mergedImage.image))
                != EXIT_SUCCESS) {
            break;
        }

        break;
    }

    // Cleanup and exit
    free_image_resources(&mergedImage);
    return status;
}

int check_valid_file_type(const char* const type, const char* filePath)
{
    // Check path ends with expected file type string
    if (!ends_with(type, filePath)) {

        // Prints error message to stderr
        fputs(fileTypeMessage, stderr);
        fprintf(stderr, unexpectedArgMessage, filePath, type);
        return -1;
    }

    return EXIT_SUCCESS;
}

void handle_image_rotation(BMP* bmpImage, const long nRotations)
{
    long rotMode = (nRotations % 4);
    rotMode = (rotMode < 0) ? (rotMode + 4) : rotMode;

    Image* output = NULL;

    switch (rotMode) {
    case 0:
        break;

    case 1:
        output = rotate_image_clockwise(bmpImage->image);
        break;

    case 2:
        if (flip_image(bmpImage->image) == -1) {
            break;
        }
        reverse_image(bmpImage->image);
        break;

    case 3:
        output = rotate_image_anticlockwise(bmpImage->image);
        break;

    default:
        break;
    }

    // Free memory in cases where new image is created
    if (output != NULL) {
        free_image(&(bmpImage->image));
        bmpImage->image = output;
    }
}
