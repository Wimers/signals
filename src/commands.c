#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <getopt.h>
#include "commands.h"
#include "utils.h"
#include "fileParsing.h"
#include "filters.h"
#include "imageEditing.h"
#include "errors.h"

// Allows for terminal rendering via SDL
#ifdef ENABLE_SDL
#include "renderSDL.h"
#endif

// These should be reordered for performance once finalised
typedef struct {
    bool input;
    char* inputFilePath;
    bool output;
    char* outputFilePath;
    bool help;
    bool header;
    bool print;
    bool invert;
    uint8_t filters;

    // Hue
    bool hue;
    int hueRed;
    int hueBlue;
    int hueGreen;

    bool grayscale;
    bool flip;
    uint8_t cutoff;
    bool combine;
    char* combineFilePath;
    size_t glitch;
    uint8_t average;
    bool contrast;
    float contrastFactor;
    bool swap;
    long rotations;
    bool reverse;
    bool melt;
    int32_t meltOffset;

    // Scale
    bool scale;
    float scaleRed;
    float scaleGreen;
    float scaleBlue;

    // Scale Strict
    bool scaleStrict;
    float strictRed;
    float strictGreen;
    float strictBlue;

    bool merge;
    char* mergeFilePath;
    size_t blur;
    bool encode;
    char* encodeFilePath;
    bool experimental;
    bool transpose;
} UserInput;

int check_valid_file_type(const char* const type, const char* filePath)
{
    // Check path ends with expected file type string
    if (!ends_with(type, filePath)) {

        // Prints error message to stderr
        fprintf(stderr, fileTypeMessage, type);
        fprintf(stderr, unexpectedArgMessage, filePath, type);
        return -1;
    }

    return EXIT_SUCCESS;
}

int status;

typedef enum {
    INVALID = -1,

    // I/O:
    INPUT = 'i',
    OUTPUT = 'o',
    MERGE = 'm',
    COMBINE = 'c',
    DUMP = 'd',
    PRINT = 'p',
    ENCODE = 'e',

    // Colours & Channels:
    FILTERS = 'f',
    HUE = 'h',
    GRAYSCALE = 'g',
    AVERAGE = 'a',
    INVERT = 'v',
    SWAP = 's',

    // Geometry & Orientation:
    ROTATE = 'r',
    TRANSPOSE = 't',
    REVERSE = 'R',
    FLIP = 'F',

    // Brightness & Contrast:
    CONTRAST = 'C',
    BRIGHTNESS_CUT = 'b',
    SCALE_STRICT = 'T',

    // Advanced Effects:
    MELT = 'M',
    GLITCH = 'G',
    SCALE = 'S',
    BLUR = 'B',

    EXPERIMENTAL = 'E',
} Flag;

constexpr char optstring[]
        = "i:o:m:c:e:f:h:r:C:b:T:M:G:S:B:dpgavstRFE"; // Defined program flags

static struct option const longOptions[] = {
        {"input", required_argument, NULL, INPUT},
        {"output", required_argument, NULL, OUTPUT},
        {"dump", no_argument, NULL, DUMP},
        {"print", no_argument, NULL, PRINT},
        {"filter", required_argument, NULL, FILTERS},
        {"hue", required_argument, NULL, HUE},
        {"grayscale", no_argument, NULL, GRAYSCALE},
        {"invert", no_argument, NULL, INVERT},
        {"flip", no_argument, NULL, FLIP},
        {"brightness-cut", required_argument, NULL, BRIGHTNESS_CUT},
        {"combine", required_argument, NULL, COMBINE},
        {"glitch", required_argument, NULL, GLITCH},
        {"average", no_argument, NULL, AVERAGE},
        {"contrast", required_argument, NULL, CONTRAST},
        {"swap", no_argument, NULL, SWAP},
        {"rotate", required_argument, NULL, ROTATE},
        {"transpose", no_argument, NULL, TRANSPOSE},
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

static UserInput store = {0};
static UserInput* userInput = &store;

typedef struct {
    const char code;
    const char* const name;
    const char* const usage;
    const char* const desc;
    const char* const examples;
} GetHelp;

typedef struct {
    int (*verify)(void);
    int (*run)(void*);
    const GetHelp help;
} Command;

typedef struct {
    const char* const name;
    const char code;
    const Command cmd;
} Entry;

static uint64_t activeCommands = 0;
static int32_t cmdOrder[64] = {INVALID};
static uint32_t cmdCount = 0;

/* COMMAND LINE ARGUMENT VERIFICATION COMMANDS
 * -------------------------------------------
 */

static int verify_dump(void)
{
    userInput->header = true;
    return 0;
}

static int verify_print(void)
{
    userInput->print = true;
    return 0;
}

static int verify_input(void)
{
    userInput->input = true;
    userInput->inputFilePath = optarg;
    return 0;
}

static int verify_output(void)
{
    userInput->output = true;
    userInput->outputFilePath = optarg;
    return 0;
}

static int verify_filter(void)
{
    uint8_t colourBitVect = 0;

    for (const char* s = optarg; *s != '\0'; s++) {

        // Sets bit in bit vector to indicate presence of colour to
        // filter. Both 'r' and 'R' are represented by the same bit.
        switch (*s) {
        case 'r':
        case 'R':
            colourBitVect |= 1; // Bit 0
            break;

        case 'g':
        case 'G':
            colourBitVect |= 2; // Bit 1
            break;

        case 'b':
        case 'B':
            colourBitVect |= 4; // Bit 2
            break;

        default:
            userInput->filters = 0;
            status = EXIT_INVALID_PARAMETER;
            goto esc;
        }
    }
esc:
    if (status) {
        fprintf(stderr, invalidFilterColourMessage, optarg);
        printf("See \'signals help filter\'\n");
    } else {
        userInput->filters = colourBitVect;
    }

    return status;
}

static int verify_hue(void)
{
    int* hueScales = separate_to_int_array(optarg, ',', 3);
    if (hueScales == NULL) {
        fprintf(stderr, invalidVal, optarg);
        printf("See \'signals help hue\'\n");
        return EXIT_INVALID_PARAMETER;
    }

    userInput->hue = true;
    userInput->hueRed = hueScales[0];
    userInput->hueGreen = hueScales[1];
    userInput->hueBlue = hueScales[2];
    free(hueScales);

    return 0;
}

static int verify_grayscale(void)
{
    userInput->grayscale = true;
    return 0;
}

static int verify_invert(void)
{
    userInput->invert = true;
    return 0;
}

static int verify_flip(void)
{
    userInput->flip = true;
    return 0;
}

static int verify_transpose(void)
{
    userInput->transpose = true;
    return 0;
}

static int verify_brightness_cut(void)
{
    if (!(vlongB(&(userInput->cutoff), optarg, 0, UINT8_MAX, uint8_t))) {
        fprintf(stderr, invalidVal, optarg);
        printf("See \'signals help brightness-cut\'\n");
        return EXIT_INVALID_PARAMETER;
    }
    return 0;
}

static int verify_combine(void)
{
    userInput->combine = true;
    userInput->combineFilePath = optarg;
    return 0;
}

static int verify_merge(void)
{
    userInput->merge = true;
    userInput->mergeFilePath = optarg;
    return 0;
}

static int verify_encode(void)
{
    userInput->encode = true;
    userInput->encodeFilePath = optarg;
    return 0;
}

static int verify_glitch(void)
{
    if (!(vlongB(&(userInput->glitch), optarg, 1, INT32_MAX, size_t))) {
        fprintf(stderr, invalidVal, optarg);
        printf("See \'signals help glitch\'\n");
        return EXIT_INVALID_PARAMETER;
    }
    return 0;
}

static int verify_average(void)
{
    userInput->average = true;
    return 0;
}

static int verify_contrast(void)
{
    float* arg = separate_to_float_array(optarg, ',', 1);
    if (!arg) {
        fprintf(stderr, invalidVal, optarg);
        printf("See \'signals help contrast\'\n");
        return EXIT_INVALID_PARAMETER;
    }

    userInput->contrast = true;
    userInput->contrastFactor = arg[0];
    free(arg);
    return 0;
}

static int verify_swap(void)
{
    userInput->swap = true;
    return 0;
}

static int verify_rotate(void)
{
    if (!(vlongB(&(userInput->rotations), optarg, LONG_MIN, LONG_MAX, long))) {
        fprintf(stderr, invalidVal, optarg);
        printf("See \'signals help rotate\'\n");
        return EXIT_INVALID_PARAMETER;
    }
    return 0;
}

static int verify_reverse(void)
{
    userInput->reverse = true;
    return 0;
}

static int verify_melt(void)
{
    bool success = true;

    if (!(vlongB(&(userInput->meltOffset), optarg, INT32_MIN, INT32_MAX,
                int32_t))) {
        success = false;
    }

    // Could remove since this would have no effect if activated
    if (userInput->meltOffset == 0) {
        success = false;
    }

    if (success == false) {
        fprintf(stderr, invalidVal, optarg);
        printf("See \'signals help melt\'\n");
        return EXIT_INVALID_PARAMETER;
    }
    return 0;
}

static int verify_scale(void)
{
    float* scaleArgs = separate_to_float_array(optarg, ',', 3);
    if (!scaleArgs) {
        fprintf(stderr, invalidVal, optarg);
        printf("See \'signals help scale\'\n");
        return EXIT_INVALID_PARAMETER;
    }

    userInput->scale = true;
    userInput->scaleRed = scaleArgs[0];
    userInput->scaleGreen = scaleArgs[1];
    userInput->scaleBlue = scaleArgs[2];
    free(scaleArgs);

    return 0;
}

static int verify_scale_strict(void)
{
    float* scaleStrictArgs = separate_to_float_array(optarg, ',', 3);
    if (!scaleStrictArgs) {
        fprintf(stderr, invalidVal, optarg);
        printf("See \'signals help scale-strict\'\n");
        return EXIT_INVALID_PARAMETER;
    }

    userInput->scaleStrict = true;
    userInput->strictRed = scaleStrictArgs[0];
    userInput->strictGreen = scaleStrictArgs[1];
    userInput->strictBlue = scaleStrictArgs[2];
    free(scaleStrictArgs);

    return 0;
}

static int verify_blur(void)
{
    if (!(vlongB(&(userInput->blur), optarg, 1, INT32_MAX, size_t))) {
        fprintf(stderr, invalidVal, optarg);
        printf("See \'signals help blur\'\n");
        return EXIT_INVALID_PARAMETER;
    }
    return 0;
}

static int verify_experimental(void)
{
    userInput->experimental = true;
    return 0;
}

/* EXECUTION COMMANDS
 * ------------------
 */

static int run_input(void* obj)
{
    (void)obj;
    return EXIT_SUCCESS;
}

static int run_output(void* obj)
{
    BMP* bmpImage = (BMP*)obj;

    if (userInput->encode) {
        if (write_bmp_with_header_provided(bmpImage, userInput->outputFilePath,
                    userInput->encodeFilePath)
                == -1) {
            status = EXIT_OUTPUT_FILE_ERROR;
        }
    } else {
        if (write_bmp_with_header_provided(
                    bmpImage, userInput->outputFilePath, NULL)
                == -1) {
            status = EXIT_OUTPUT_FILE_ERROR;
        }
    }
    return status;
}

static int run_merge(void* obj)
{
    BMP* bmpImage = (BMP*)obj;

    if (check_valid_file_type(fileType, userInput->mergeFilePath) == -1) {
        status = EXIT_INVALID_FILE_TYPE;
        goto failure;
    }

    // Exit if input and merge file paths match
    if (!strcmp(userInput->inputFilePath, userInput->mergeFilePath)) {
        fputs(nonUniquePathsMessage, stderr);
        status = EXIT_SAME_FILE;
        goto failure;
    }

    BMP mergedImage;
    initialise_bmp(&mergedImage);

    while (1) {
        status = open_bmp(&mergedImage, userInput->mergeFilePath);
        if (status != EXIT_SUCCESS) {
            break;
        }

        status = handle_bmp_loading(&mergedImage);
        if (status != EXIT_SUCCESS) {
            break;
        }

        status = merge_images(bmpImage->image, mergedImage.image);
        break;
    }

    // Cleanup and exit
    free_image_resources(&mergedImage);
    if (status != EXIT_SUCCESS) {
        goto failure;
    }
    return status;

failure:
    fprintf(stderr, "Merge failed\n");
    return status;
}

static int run_combine(void* obj)
{
    BMP* bmpImage = (BMP*)obj;

    if (check_valid_file_type(fileType, userInput->combineFilePath) == -1) {
        status = EXIT_INVALID_FILE_TYPE;
        goto failure;
    }

    // Exit if input and combine file paths match
    if (!strcmp(userInput->inputFilePath, userInput->combineFilePath)) {
        fputs(nonUniquePathsMessage, stderr);
        status = EXIT_SAME_FILE;
        goto failure;
    }

    BMP combinedImage;
    initialise_bmp(&combinedImage);

    while (1) {
        status = open_bmp(&combinedImage, userInput->combineFilePath);
        if (status != EXIT_SUCCESS) {
            break;
        }

        status = handle_bmp_loading(&combinedImage);
        if (status != EXIT_SUCCESS) {
            break;
        }

        status = combine_images(bmpImage->image, combinedImage.image);
        break;
    }

    // Cleanup and exit
    free_image_resources(&combinedImage);
    if (status != EXIT_SUCCESS) {
        goto failure;
    }
    return status;

failure:
    fprintf(stderr, "Combine failed\n");
    return status;
}

static int run_dump(void* obj)
{
    BMP* bmpImage = (BMP*)obj;
    dump_headers(bmpImage);
    return EXIT_SUCCESS;
}

static int run_print(void* obj)
{
    BMP* bmpImage = (BMP*)obj;

    if (flip_image(bmpImage->image) == -1) {
        status = EXIT_FAILURE; // FIX
        return status;
    }

#ifdef ENABLE_SDL
    status = render_image(bmpImage->image);
    return status;
#endif
    print_image_to_terminal(bmpImage->image);
    return EXIT_SUCCESS;
}

// Encoding logic is currently handled inside "run_output"
static int run_encode(void* obj)
{
    (void)obj;
    return EXIT_SUCCESS;
}

static int run_filter(void* obj)
{
    BMP* bmpImage = (BMP*)obj;

    const uint8_t index = userInput->filters & 0x07;

    typedef void (*Function)(void);
    static const Function filterMap[8] = {
            NULL,
            (Function)&filter_red,
            (Function)&filter_green,
            (Function)&filter_red_green,
            (Function)&filter_blue,
            (Function)&filter_red_blue,
            (Function)&filter_green_blue,
            (Function)&filter_all,
    };

    typedef void (*ColourFilter)(Image* image);
    ColourFilter filter = (ColourFilter)(filterMap[index]);

    if (filter == NULL) { // Check this is even possible
        status = EXIT_FAILURE; // FIX
        return status;
    }

    filter(bmpImage->image);
    return EXIT_SUCCESS;
}

static int run_hue(void* obj)
{
    BMP* bmpImage = (BMP*)obj;
    apply_hue(bmpImage->image, userInput->hueRed, userInput->hueGreen,
            userInput->hueBlue);
    return EXIT_SUCCESS;
}

static int run_grayscale(void* obj)
{
    BMP* bmpImage = (BMP*)obj;
    gray_filter(bmpImage->image);
    return EXIT_SUCCESS;
}

static int run_average(void* obj)
{
    BMP* bmpImage = (BMP*)obj;
    average_pixels(bmpImage->image);
    return EXIT_SUCCESS;
}

static int run_invert(void* obj)
{
    BMP* bmpImage = (BMP*)obj;
    invert_colours(bmpImage->image);
    return EXIT_SUCCESS;
}

static int run_swap(void* obj)
{
    BMP* bmpImage = (BMP*)obj;
    swap_red_blue(bmpImage->image);
    return EXIT_SUCCESS;
}

static int run_contrast(void* obj)
{
    BMP* bmpImage = (BMP*)obj;
    contrast_effect(bmpImage->image, userInput->contrastFactor);
    return EXIT_SUCCESS;
}

static int run_brightness_cut(void* obj)
{
    BMP* bmpImage = (BMP*)obj;
    brightness_cut_filter(bmpImage->image, userInput->cutoff);
    return EXIT_SUCCESS;
}

static int run_scale_strict(void* obj)
{
    BMP* bmpImage = (BMP*)obj;
    colour_scaler_strict(bmpImage->image, userInput->strictRed,
            userInput->strictGreen, userInput->strictBlue);
    return EXIT_SUCCESS;
}

static int run_melt(void* obj)
{
    BMP* bmpImage = (BMP*)obj;
    if (melt(bmpImage, userInput->meltOffset) == -1) {
        fprintf(stderr, "Melt failed\n");
        status = EXIT_FAILURE; // FIX
        return status;
    }
    return EXIT_SUCCESS;
}

static int run_glitch(void* obj)
{
    BMP* bmpImage = (BMP*)obj;
    if (glitch_effect(bmpImage->image, userInput->glitch) == -1) {
        status = EXIT_OUT_OF_BOUNDS;
        return status;
    }
    return EXIT_SUCCESS;
}

static int run_scale(void* obj)
{
    BMP* bmpImage = (BMP*)obj;
    colour_scaler(bmpImage->image, userInput->scaleRed, userInput->scaleGreen,
            userInput->scaleBlue);
    return EXIT_SUCCESS;
}

static int run_blur(void* obj)
{
    BMP* bmpImage = (BMP*)obj;
    Image* blurred = even_faster_image_blur(bmpImage->image, userInput->blur);

    if (blurred == NULL) {
        fprintf(stderr, "Blurring failed\n");
        status = EXIT_FAILURE; // FIX
        return status;
    }

    free_image(&(bmpImage->image));
    bmpImage->image = blurred;
    return EXIT_SUCCESS;
}

static int run_rotate(void* obj)
{
    BMP* bmpImage = (BMP*)obj;

    // Handle image rotation
    long rotMode = (userInput->rotations % 4);
    rotMode = (rotMode < 0) ? (rotMode + 4) : rotMode;

    Image* output = NULL;

    switch (rotMode) {
    case 0:
        break;

    case 1: {
        output = rotate_image_clockwise(bmpImage->image);
        break;
    }

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
        unreachable();
    }

    // Free memory in cases where new image is created
    if (output != NULL) {
        free_image(&(bmpImage->image));
        bmpImage->image = output;
    }

    if (bmpImage->image == NULL) {
        fprintf(stderr, "Rotation failed\n");
        status = EXIT_FAILURE; // FIX
        return status;
    }

    if (userInput->rotations & 1) { // If number of rotations is odd
        const int32_t temp = (bmpImage->infoHeader).bitmapWidth;
        (bmpImage->infoHeader).bitmapWidth
                = (bmpImage->infoHeader).bitmapHeight;
        (bmpImage->infoHeader).bitmapHeight = temp;
    }
    return EXIT_SUCCESS;
}

static int run_transpose(void* obj)
{
    BMP* bmpImage = (BMP*)obj;
    Image* transpose = transpose_image(bmpImage->image);

    free_image(&(bmpImage->image));
    bmpImage->image = transpose;

    // Update image dimensions
    const int32_t temp = (bmpImage->infoHeader).bitmapWidth;
    (bmpImage->infoHeader).bitmapWidth = (bmpImage->infoHeader).bitmapHeight;
    (bmpImage->infoHeader).bitmapHeight = temp;

    return EXIT_SUCCESS;
}

static int run_reverse(void* obj)
{
    BMP* bmpImage = (BMP*)obj;
    reverse_image(bmpImage->image);
    return EXIT_SUCCESS;
}

static int run_flip(void* obj)
{
    BMP* bmpImage = (BMP*)obj;
    if (flip_image(bmpImage->image) == -1) {
        status = EXIT_FAILURE; // FIX
        return status;
    }
    return EXIT_SUCCESS;
}

// Placeholder
static int run_experimental(void* obj)
{
    (void)obj;
    return EXIT_SUCCESS;
}

/* INITIALISE ALL COMMANDS
 * -----------------------
 */

static const Command Input = {
	.verify = verify_input,
	.run = run_input,
        .help = {
		.code = 'i',
		.name = "input",
                .usage = "--input <file>",
                .desc = "Specifies the source BMP image file to be "
                        "processed.\n\tFilename "
                        "must end in \'.bmp\'.",
                .examples = "signals -i images/beach.bmp",
	},
};

static const Command Output = {
	.verify = verify_output,
	.run = run_output,
        .help = {
		.code = 'o',
		.name = "output",
		.usage = "-i <file> --output <file>",
                .desc = "Specifies the destination path"
			"where the processed BMP will be saved.",
		.examples = "signals -i in.bmp -o out.bmp",
	},
};

static const Command Merge = {
	.verify = verify_merge,
	.run = run_merge,
        .help = {
		.code = 'm',
		.name = "merge",
                .usage = "-i <file> --merge <file>",
                .desc = "Overlays the specified file onto the input file."
			"\n\tInput and merge paths must be unique.",
		.examples = "signals -i face1.bmp -m face2.bmp -o morph.bmp",
	},
};

static const Command Combine = {
    .verify = verify_combine,
    .run = run_combine,
    .help = {
        .code = 'c',
        .name = "combine",
        .usage = "-i <file> --combine <file>",
        .desc = "Averages the pixel values of the input file and the "
		"combine file (50/50 blend).\n\tInput and combine paths "
		"must be unique.",
        .examples = "signals -i back.bmp -c front.bmp -o combined.bmp",
    },
};

static const Command Dump = {
    .verify = verify_dump,
    .run = run_dump,
    .help = {
        .code = 'd',
        .name = "dump",
        .usage = "-i <file> --dump",
        .desc = "Reads the input BMP file and prints the header "
                "metadata to the terminal.",
        .examples = "signals -i image.bmp --dump",
    },
};

static const Command Print = {
    .verify = verify_print,
    .run = run_print,
    .help = {
        .code = 'p',
        .name = "print",
        .usage = "-i <file> --print",
        .desc = "Renders the image (in colour) directly to the "
                "terminal.\n\n\tFor the best viewing experience, zoom out "
		"in your terminal.\n\tUseful for checking parameters "
		"without saving to disk.",
        .examples = "signals -i icon.bmp -p",
    },
};

static const Command Encode = {
    .verify = verify_encode,
    .run = run_encode,
    .help = {
        .code = 'e',
        .name = "encode",
        .usage = "-i <file> --encode <secrets>",
        .desc = "Reads an input file and hides the contents within the "
		"output image.",
        .examples = "signals -i cover.bmp --encode secret.txt -o cover.bmp",
    },
};

static const Command Filters = {
    .verify = verify_filter,
    .run = run_filter,
    .help = {
        .code = 'f',
        .name = "filter",
        .usage = "-i <file> --filter <channels>",
        .desc = "Isolates specific colour channels. Use 'R', 'G', or 'B' "
		" in any combination.\n\tThe channels specified will be "
		"removed from the output image.",
        .examples = "signals -i in.bmp -o out.bmp --filter rb",
    },
};

static const Command Hue = {
    .verify = verify_hue,
    .run = run_hue,
    .help = {
        .code = 'h',
        .name = "hue",
        .usage = "-i <file> --hue <R, G, B>",
        .desc = "Update...", // FIX
        .examples = "signals -i in.bmp -o out.bmp --hue \'0, 0, 100\'",
    },
};

static const Command Grayscale = {
    .verify = verify_grayscale,
    .run = run_grayscale,
    .help = {
        .code = 'g',
        .name = "grayscale",
        .usage = "-i <file> --grayscale",
        .desc = "Converts the image to grayscale using the Luma RGB "
                "coefficients.\n\tR *= 0.299, G *= 0.587, B *= 0.114.",
        .examples = "signals -i in.bmp -o bw.bmp --grayscale",
    },
};

static const Command Average = {
    .verify = verify_average,
    .run = run_average,
    .help = {
        .code = 'a',
        .name = "average",
        .usage = "-i <file> --average",
        .desc = "Converts the image to grayscale using a simple average"
                " \'(R + G + B) / 3\'.",
        .examples = "signals -i in.bmp -o avg.bmp --average",
    },
};

static const Command Invert = {
    .verify = verify_invert,
    .run = run_invert,
    .help = {
        .code = 'v', 
        .name = "invert",
        .usage = "-i <file> --invert",
        .desc = "Inverts the colours of the image (creates a negative).",
        .examples = "signals -i wb.bmp -o bw.bmp --invert",
    },
};

static const Command Swap = {
    .verify = verify_swap,
    .run = run_swap,
    .help = {
        .code = 's',
        .name = "swap",
        .usage = "-i <file> --swap",
        .desc = "Swaps the Red and Blue colour channels.",
        .examples = "signals -i in.bmp -o swapped.bmp --swap",
    },
};

static const Command Contrast = {
    .verify = verify_contrast,
    .run = run_contrast,
    .help = {
        .code = 'C',
        .name = "contrast",
        .usage = "-i <file> --contrast <factor>",
        .desc = "Increase the contrast of an image.",
        .examples = "signals -i in.bmp -o out.bmp --contrast 150",
    },
};

static const Command BrightnessCut = {
    .verify = verify_brightness_cut,
    .run = run_brightness_cut,
    .help = {
        .code = 'b',
        .name = "brightness-cut",
        .usage = "-i <file> --brightness-cut <0-255>",
        .desc = "Sets a pixel channel to 0 if it exceeds the cutoff value.",
        .examples = "signals -i in.bmp -o cut.bmp --brightness-cut 200",
    },
};

static const Command ScaleStrict = {
    .verify = verify_scale_strict,
    .run = run_scale_strict,
    .help = {
        .code = 'T',
        .name = "scale-strict",
        .usage = "-i <file> --scale-strict <float>",
        .desc = "Multiplies colour intensity by a constant "
                "factor.\n\tIntensity is clamped above by UINT8_MAX.",
        .examples = "signals -i in.bmp -o strict.bmp --scale-strict 0.5",
    },
};

static const Command Melt = {
    .verify = verify_melt,
    .run = run_melt,
    .help = {
        .code = 'M',
        .name = "melt",
        .usage = "-i <file> --melt <offset>",
        .desc = "Applies a pixel sorting 'melt' effect.\n\tUse negative "
		"values to melt upwards.",
        .examples = "signals -i in.bmp -o melted.bmp --melt 50",
    },
};

static const Command Glitch = {
    .verify = verify_glitch,
    .run = run_glitch,
    .help = {
        .code = 'G',
        .name = "glitch",
        .usage = "-i <file> --glitch <offset>",
        .desc = "Applies a horizontal shift effect to red/blue channels "
		"based on the offset provided.\n\tThe offset must be a "
		"positive integer, and also within the horizontal "
		"dimensions of the input image.",
        .examples = "signals -i in.bmp -o glitch.bmp --glitch 20",
    },
};

static const Command Scale = {
    .verify = verify_scale,
    .run = run_scale,
    .help = {
        .code = 'S',
        .name = "scale",
        .usage = "-i <file> --scale <float>",
        .desc = "Scales colour intensity, allowing 8-bit integer overflow "
		"for some very interesting effets :)",
        .examples = "signals -i in.bmp -o trippy.bmp --scale 2.5",
    },
};

static const Command Blur = {
    .verify = verify_blur,
    .run = run_blur,
    .help = {
        .code = 'B',
        .name = "blur",
        .usage = "-i <file> --blur <radius>",
        .desc = "Applies a box blur with the specified radius to the image.",
        .examples = "signals -i in.bmp -o blurred.bmp --blur 5",
    },
};

static const Command Rotate = {
    .verify = verify_rotate,
    .run = run_rotate,
    .help = {
        .code = 'r',
        .name = "rotate",
        .usage = "-i <file> --rotate <N>",
        .desc = "Rotates the image 90° clockwise N times.\n\tNegative "
                "numbers rotate anti-clockwise.",
        .examples = "signals -i in.bmp -o rotated.bmp --rotate 1",
    },
};

static const Command Transpose = {
    .verify = verify_transpose,
    .run = run_transpose,
    .help = {
        .code = 't',
        .name = "transpose",
        .usage = "-i <file> --transpose",
        .desc = "Transposes the input image.",
        .examples = "signals -i in.bmp -o out.bmp --transpose",
    },
};

static const Command Reverse = {
    .verify = verify_reverse,
    .run = run_reverse,
    .help = {
        .code = 'R',
        .name = "reverse",
        .usage = "-i <file> --reverse",
        .desc = "Mirrors the image horizontally.",
        .examples = "signals -i in.bmp -o mirror.bmp --reverse",
    },
};

static const Command Flip = {
    .verify = verify_flip,
    .run = run_flip,
    .help = {
        .code = 'F',
        .name = "flip",
        .usage = "-i <file> --flip",
        .desc = "Flips the image vertically.",
        .examples = "signals -i in.bmp -o flipped.bmp --flip",
    },
};

static const Command Experimental = {
    .verify = verify_experimental,
    .run = run_experimental,
    .help = {
        .code = 'E',
        .name = "experimental",
        .usage = "-i <file> --experimental",
        .desc = "Runs a specific experimental chain of effects.",
        .examples = "signals -i in.bmp -o exp.bmp --experimental",
    },
};

static const Entry CmdRegistry[] = {
        {"input", INPUT, Input}, {"output", OUTPUT, Output},
        {"dump", DUMP, Dump}, {"print", PRINT, Print},
        {"filter", FILTERS, Filters}, {"hue", HUE, Hue},
        {"grayscale", GRAYSCALE, Grayscale}, {"invert", INVERT, Invert},
        {"flip", FLIP, Flip}, {"brightness-cut", BRIGHTNESS_CUT, BrightnessCut},
        {"combine", COMBINE, Combine}, {"glitch", GLITCH, Glitch},
        {"average", AVERAGE, Average}, {"contrast", CONTRAST, Contrast},
        {"swap", SWAP, Swap}, {"rotate", ROTATE, Rotate},
        {"transpose", TRANSPOSE, Transpose}, {"reverse", REVERSE, Reverse},
        {"melt", MELT, Melt}, {"scale", SCALE, Scale},
        {"scale-strict", SCALE_STRICT, ScaleStrict}, {"merge", MERGE, Merge},
        {"blur", BLUR, Blur}, {"encode", ENCODE, Encode},
        {"experimental", EXPERIMENTAL, Experimental},
        {NULL, INVALID, {0}}, // INVALID
};

int parse_user_commands(const int argc, char** argv)
{
    Flag opt;

    while ((opt = getopt_long(argc, argv, optstring, longOptions, NULL))
            != -1) {
        int32_t i = 0;

        while (1) {
            if ((CmdRegistry[i]).code == opt) {
                break;
            }

            if ((CmdRegistry[i]).code == INVALID) { // INVALID
                fputs(userHelpPrompt, stdout);
                return EXIT_NON_EXISTENT_COMMAND;
            }

            i++;
        }

        const Command* cmd = &((CmdRegistry[i]).cmd);
        const uint64_t mask = (1 << i);

        if (activeCommands & mask) { // Check if this command has
                                     // already been parsed
            return EXIT_FAILURE; // FIX
        }

        int success = cmd->verify();
        if (success != 0) {
            return success;
        }

        // Set commands to active
        activeCommands |= mask;
        cmdOrder[cmdCount] = i;
        cmdCount++;
    }

    if (optind < argc) {
        fprintf(stderr, invalidCmdMessage, argv[optind]);
        return EXIT_TOO_MANY_ARGS;
    }

    // All options parses successfully
    return EXIT_SUCCESS;
}

int handle_commands(void)
{
    // An input file is required all non-help commands
    if (!(userInput->input)) {
        fprintf(stderr, "signals: no input file provided. ");
        printf(userHelpPrompt);
        return EXIT_MISSING_INPUT_FILE;
    }

    // Check the file type of supplied file path
    if (check_valid_file_type(fileType, userInput->inputFilePath) == -1) {
        return EXIT_INVALID_FILE_TYPE;
    }

    // Initialise struct to store BMP data
    BMP bmpImage;
    initialise_bmp(&bmpImage);

    // Attempt to open the BMP and read file headers
    status = open_bmp(&bmpImage, userInput->inputFilePath);
    if (status != EXIT_SUCCESS) {
        goto cleanup;
    }

    if (userInput->header) {
        Dump.run(&bmpImage);
    }

    // Attempt to load pixel data from file into bmpImage struct
    status = handle_bmp_loading(&bmpImage);
    if (status != EXIT_SUCCESS) {
        goto cleanup;
    }

    for (uint32_t i = 0; i < cmdCount; i++) {
        if (cmdOrder[i] == INVALID) {
            goto cleanup;
        }

        const Command* cmd = &((CmdRegistry[cmdOrder[i]]).cmd);
        const char* const name = (cmd->help).name;
        if (!strcmp(name, "dump") || !strcmp(name, "input")
                || !strcmp(name, "output") || !strcmp(name, "print")) {
            fprintf(stderr, "Ignoring \'%s\'\n", name);
            continue;
        }

        status = cmd->run(&bmpImage);
        if (status != EXIT_SUCCESS) {
            goto cleanup;
        }
    }

    if (userInput->output) {
        status = Output.run(&bmpImage);
        if (status != EXIT_SUCCESS) {
            goto cleanup;
        }
    }

    if (userInput->print) {
        status = Print.run(&bmpImage);
        // goto cleanup
    }

cleanup:
    // Cleanup and exit
    free_image_resources(&bmpImage);
    return status;
}

static void get_cmd_help(const Command* cmd)
{
    const GetHelp* helper = &(cmd->help);

    fprintf(stdout,
            "\n"
            "NAME\n"
            "\t-%c, --%s\n\n"

            "USAGE\n"
            "\t%s\n\n"

            "DESCRIPTION\n"
            "\t%s\n\n"

            "EXAMPLES\n"
            "\t%s\n\n",
            helper->code, helper->name, helper->usage, helper->desc,
            helper->examples);
}

int command_list(const char* command)
{
    int i = 0;

    while (1) {
        const char* setting = (CmdRegistry[i]).name;

        if (setting == NULL) { // command is not registered
            fprintf(stderr, invalidCmdMessage, command);
            return INVALID;
        }

        if (!(strcmp(setting, command))) {
            break;
        }

        i++;
    }

    get_cmd_help(&((CmdRegistry[i]).cmd));
    return EXIT_SUCCESS;
}
