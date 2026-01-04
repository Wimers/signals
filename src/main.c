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
constexpr char helpMessage[]
        = "Usage: signals <option> [--input <file>] ...\n"
          "\n"
          "Help:\n"
          "  \'signals help\'              - Show this help message\n"
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
          "\n"
          "Geometry & Orientation:\n"
          "  -r, --rotate <N>            - Rotate image 90° clockwise N "
          "times\n"
          "  -t, --transpose             - Transposes the image\n"
          "  -R, --reverse               - Reverse image horizontally\n"
          "  -F, --flip                  - Flip image vertically times\n"
          "\n"

          "Brightness & Contrast:\n"
          "  -C, --contrast <val>        - Adjust contrast factor (0-255)\n"
          "  -b, --brightness-cut <val>  - Cuts pixel component if value "
          "exceeds cutoff (0-255)\n"
          "  -D, --dim <val>             - Reduce brightness by value "
          "(0-255)\n"
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
          "  -E, --experimental          - Try out an experimental feature!\n"
          "\n"
          "See \'signals help <command>\' to read about a specific command.\n";

constexpr char fileTypeMessage[] = "Input must be a \'%s\' file.\n";
constexpr char invalidCmdMessage[]
        = "signals: \'%s\' is not a valid command. See \'signals help\'\n";
constexpr char userHelpPrompt[] = "See \'signals help\'.\n";
constexpr char nonUniquePathsMessage[]
        = "signals: input and combine file paths must be unique!\n";
constexpr char fileType[] = ".bmp";
constexpr char invalidVal[] = "signals: invalid value \'%s\'\n";

// Error messages
constexpr char unexpectedArgMessage[] = "Got \'%s\', expected \'%s\'\n";
constexpr char gotStrMessage[] = "    Got \'%s\'.\n";
constexpr char invalidFilterColourMessage[]
        = "signals: filter colour/s \'%s\' are invalid, must be RGB "
          "characters.\n";

// Assorted constant chars
const char* const readMode = "rb";
const char* const writeMode = "wb";

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
        = "i:o:m:c:e:f:r:C:b:D:T:M:G:S:B:dpgavstRFE"; // Defined program flags

static struct option const longOptions[] = {
        {"input", required_argument, NULL, INPUT},
        {"output", required_argument, NULL, OUTPUT},
        {"dump", no_argument, NULL, DUMP},
        {"print", no_argument, NULL, PRINT},
        {"filter", required_argument, NULL, FILTERS},
        {"grayscale", no_argument, NULL, GRAYSCALE},
        {"invert", no_argument, NULL, INVERT},
        {"flip", no_argument, NULL, FLIP},
        {"brightness-cut", required_argument, NULL, BRIGHTNESS_CUT},
        {"combine", required_argument, NULL, COMBINE},
        {"glitch", required_argument, NULL, GLITCH},
        {"average", no_argument, NULL, AVERAGE},
        {"contrast", required_argument, NULL, CONTRAST},
        {"dim", required_argument, NULL, DIM},
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

int main(const int argc, char* argv[])
{
    // Check if user did not supply any arguments
    if (argc <= 1) {
        help_message();
        exit(EXIT_NO_ARGUMENTS);
    }

    if (any_empty_args(argc, argv)) {
        help_message();
        exit(EXIT_EMPTY_ARG);
    }

    // Initialise
    UserInput userInput;
    (void)memset(&userInput, 0, sizeof(userInput));

    if (!(strcmp(argv[1], "help"))) {
        if (argc == 2) {
            help_message();

        } else if (argc == 3) { // Print output for branch
                                // handled by command_list
            char* cmd = argv[2];
            if (command_list(cmd) == -1) {
                exit(EXIT_NON_EXISTENT_COMMAND);
            }
        } else {
            help_message();
            exit(EXIT_TOO_MANY_ARGS);
        }

        return EXIT_SUCCESS;
    }

    int status;
    if ((status = parse_user_commands(argc, argv, &userInput))
            != EXIT_SUCCESS) {
        exit(status);
    }

    exit(handle_commands(&userInput));
}

void help_message(void)
{
    fputs(helpMessage, stdout);
}

bool any_empty_args(const int argc, char** argv)
{
    for (int i = 0; i < argc; i++) {
        if (argv[i][0] == '\0') {
            return true;
        }
    }

    return false;
}

// Helper function to format the output consistently
void print_cmd_help(const char* name, const char shortOpt, const char* desc,
        const char* usage, const char* examples)
{
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
            shortOpt, name, usage, desc, examples);
}

int command_list(const char* command)
{
    int i = 0;
    int status = INVALID;

    while (1) {
        const char* setting = (longOptions[i]).name;

        if (setting == NULL) { // command is not registered
            goto invalid;
        }

        if (!(strcmp(setting, command))) {
            status = (longOptions[i]).val;
            break;
        }

        i++;
    }

    const char cmd = (char)status;

    switch (cmd) {
    case INPUT:
        print_cmd_help("input", cmd,
                "Specifies the source BMP image file to be "
                "processed.\n\tFilename "
                "must end in \'.bmp\'.",
                "--input <file>", "signals -i images/beach.bmp");
        break;

    case OUTPUT:
        print_cmd_help("output", cmd,
                "Specifies the destination path where the processed BMP will "
                "be saved.",
                "-i <file> --output <file>", "signals -i in.bmp -o out.bmp");
        break;

    case MERGE:
        print_cmd_help("merge", cmd,
                "Overlays the specified file onto the input file.\n\tInput "
                "and merge paths must be unique.",
                "-i <file> --merge <file>",
                "signals -i face1.bmp -m face2.bmp -o morph.bmp");
        break;

    case COMBINE:
        print_cmd_help("combine", cmd,
                "Averages the pixel values of the input file and the combine "
                "file (50/50 blend).\n\tInput and combine paths must be "
                "unique.",
                "-i <file> --combine <file>",
                "signals -i back.bmp -c front.bmp -o combined.bmp");
        break;

    case DUMP:
        print_cmd_help("dump", cmd,
                "Reads the input BMP file and prints the header "
                "metadata to the terminal.",
                "-i <file> --dump", "signals -i image.bmp --dump");
        break;

    case PRINT:
        print_cmd_help("print", cmd,
                "Renders the image (in colour) directly to the "
                "terminal.\n\n\tFor the best viewing experience, zoom out in "
                "your terminal.\n\tUseful for checking parameters without "
                "saving to disk.",
                "-i <file> --print", "signals -i icon.bmp -p");
        break;

    case ENCODE:
        print_cmd_help("encode", cmd,
                "Reads an input file and hides the contents within the output "
                "image.",
                "-i <file> --encode <secrets>",
                "signals -i cover.bmp --encode secret.txt -o cover.bmp");
        break;

    case FILTERS:
        print_cmd_help("filter", cmd,
                "Isolates specific colour channels. Use 'R', 'G', or 'B' in "
                "any combination.\n\tThe channels specified will be removed "
                "from the output image.",
                "-i <file> --filter <channels>",
                "signals -i in.bmp -o out.bmp --filter rb");
        break;

    case GRAYSCALE:
        print_cmd_help("grayscale", cmd,
                "Converts the image to grayscale using the Luma RGB "
                "coefficients.\n\tR *= 0.299, G *= 0.587, B *= 0.114.",
                "-i <file> --grayscale",
                "signals -i in.bmp -o bw.bmp --grayscale");
        break;

    case AVERAGE:
        print_cmd_help("average", cmd,
                "Converts the image to grayscale using a simple average"
                " \'(R + G + B) / 3\'.",
                "-i <file> --average",
                "signals -i in.bmp -o avg.bmp --average");
        break;

    case INVERT:
        print_cmd_help("invert", cmd,
                "Inverts the colours of the image (creates a negative).",
                "-i <file> --invert", "signals -i wb.bmp -o bw.bmp --invert");
        break;

    case SWAP:
        print_cmd_help("swap", cmd, "Swaps the Red and Blue colour channels.",
                "-i <file> --swap", "signals -i in.bmp -o swapped.bmp --swap");
        break;

    case CONTRAST:
        print_cmd_help("contrast", cmd, "Increase the contrast of an image.",
                "-i <file> --contrast <0-255>",
                "signals -i in.bmp -o out.bmp --contrast 150");
        break;

    case BRIGHTNESS_CUT:
        print_cmd_help("brightness-cut", cmd,
                "Sets a pixel channel to 0 if it exceeds the cutoff value.",
                "-i <file> --brightness-cut <0-255>",
                "signals -i in.bmp -o cut.bmp --brightness-cut 200");
        break;

    case DIM:
        print_cmd_help("dim", cmd,
                "Reduces the brightness of the image by a constant.",
                "-i <file> --dim <0-255>",
                "signals -i in.bmp -o dimmed.bmp --dim 50");
        break;

    case SCALE_STRICT:
        print_cmd_help("scale-strict", cmd,
                "Multiplies colour intensity by a constant "
                "factor.\n\tIntensity is clamped above by UINT8_MAX.",
                "-i <file> --scale-strict <float>",
                "signals -i in.bmp -o strict.bmp --scale-strict 0.5");
        break;

    case MELT:
        print_cmd_help("melt", cmd,
                "Applies a pixel sorting 'melt' effect.\n\tUse negative values "
                "to melt upwards.",
                "-i <file> --melt <offset>",
                "signals -i in.bmp -o melted.bmp --melt 50");
        break;

    case GLITCH:
        print_cmd_help("glitch", cmd,
                "Applies a horizontal shift effect to red/blue channels based "
                "on the offset provided.\n\tThe offset must be a positive "
                "integer, and also within the horizontal dimensions of the "
                "input image.",
                "-i <file> --glitch <offset>",
                "signals -i in.bmp -o glitch.bmp --glitch 20");
        break;

    case SCALE:
        print_cmd_help("scale", cmd,
                "Scales colour intensity, allowing 8-bit integer overflow for "
                "some very interesting effets :)",
                "-i <file> --scale <float>",
                "signals -i in.bmp -o trippy.bmp --scale 2.5");
        break;

    case BLUR:
        print_cmd_help("blur", cmd,
                "Applies a box blur to the image using the specified radius.",
                "-i <file> --blur <radius>",
                "signals -i in.bmp -o blurred.bmp --blur 5");
        break;

    case ROTATE:
        print_cmd_help("rotate", cmd,
                "Rotates the image 90° clockwise N times.\n\tNegative "
                "numbers rotate anti-clockwise.",
                "-i <file> --rotate <N>",
                "signals -i in.bmp -o rotated.bmp --rotate 1");
        break;

    case TRANSPOSE:
        print_cmd_help("transpose", cmd, "Transposes the input image.",
                "-i <file> --transpose",
                "signals -i in.bmp -o out.bmp --transpose");
        break;

    case REVERSE:
        print_cmd_help("reverse", cmd, "Mirrors the image horizontally.",
                "-i <file> --reverse",
                "signals -i in.bmp -o mirror.bmp --reverse");
        break;

    case FLIP:
        print_cmd_help("flip", cmd, "Flips the image vertically.",
                "-i <file> --flip", "signals -i in.bmp -o flipped.bmp --flip");
        break;

    case EXPERIMENTAL:
        print_cmd_help("experimental", cmd,
                "Runs a specific experimental chain of effects.",
                "-i <file> --experimental",
                "signals -i in.bmp -o exp.bmp --experimental");
        break;

    default:
    invalid:
        fprintf(stderr, invalidCmdMessage, command);
        return INVALID;
    }

    return EXIT_SUCCESS;
}

int parse_user_commands(const int argc, char** argv, UserInput* userInput)
{
    Flag opt;

    // Loop over all of the options
    while ((opt = getopt_long(argc, argv, optstring, longOptions, NULL))
            != -1) {
        switch (opt) {

        case DUMP:
            userInput->header = true;
            break;

        case PRINT:
            userInput->print = true;
            break;

        case INPUT:
            userInput->input = true;
            userInput->inputFilePath = optarg;
            break;

        case OUTPUT:
            userInput->output = true;
            userInput->outputFilePath = optarg;
            break;

        case FILTERS: {
            if (filtr_col_check(&(userInput->filters), optarg) == -1) {
                fprintf(stderr, invalidFilterColourMessage, optarg);
                printf("See \'signals help filter\'\n");
                return EXIT_INVALID_PARAMETER;
            }
            break;
        }

        case GRAYSCALE:
            userInput->grayscale = true;
            break;

        case INVERT:
            userInput->invert = true;
            break;

        case FLIP:
            userInput->flip = true;
            break;

        case TRANSPOSE:
            userInput->transpose = true;
            break;

        case BRIGHTNESS_CUT: {
            if (!(vlongB(
                        &(userInput->cutoff), optarg, 0, UINT8_MAX, uint8_t))) {
                fprintf(stderr, invalidVal, optarg);
                printf("See \'signals help brightness-cut\'\n");
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
                fprintf(stderr, invalidVal, optarg);
                printf("See \'signals help glitch\'\n");
                // glitch_offset_invalid_message(optarg); // Prints error
                // messages
                return EXIT_INVALID_PARAMETER;
            }
            break;
        }

        case AVERAGE:
            userInput->average = true;
            break;

        case CONTRAST: { // If contrast flag set, verify the required argument
            if (!(vlongB(&(userInput->contrast), optarg, 0, UINT8_MAX,
                        uint8_t))) {
                fprintf(stderr, invalidVal, optarg);
                printf("See \'signals help contrast\'\n");
                return EXIT_INVALID_PARAMETER;
            }
            break;
        }

        case DIM: { // If dim flag set, verify the required argument
            if (!(vlongB(&(userInput->dim), optarg, 0, UINT8_MAX, uint8_t))) {
                fprintf(stderr, invalidVal, optarg);
                printf("See \'signals help dim\'\n");
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
                fprintf(stderr, invalidVal, optarg);
                printf("See \'signals help rotate\'\n");
                return EXIT_INVALID_PARAMETER;
            }
            break;
        }

        case REVERSE:
            userInput->reverse = true;
            break;

        case MELT: {
            if (!verify_melt(userInput, optarg)) {
                fprintf(stderr, invalidVal, optarg);
                printf("See \'signals help melt\'\n");
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
            if (!(vlongB(&(userInput->blur), optarg, 1, INT32_MAX, size_t))) {
                fprintf(stderr, invalidVal, optarg);
                printf("See \'signals help blur\'\n");
                return EXIT_INVALID_PARAMETER;
            }
            break;
        }

        case EXPERIMENTAL:
            userInput->experimental = true;
            break;

        // If valid command supplied, or none supplied at all
        default:
            fputs(userHelpPrompt, stdout);
            return EXIT_NON_EXISTENT_COMMAND;
        }
    }

    if (optind < argc) {
        fprintf(stderr, invalidCmdMessage, argv[optind]);
        return EXIT_TOO_MANY_ARGS;
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

[[deprecated]] void glitch_offset_invalid_message(const char* arg)
{
    fputs(glitchUsageMessage, stderr);
    fputs(glitchOffsetValMessage, stderr);
    fprintf(stderr, gotStrMessage, arg);
}

int handle_commands(UserInput* userInput)
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

        if (userInput->transpose) {
            Image* transpose = transpose_image(bmpImage.image);
            free_image(&(bmpImage.image));
            bmpImage.image = transpose;

            // Update image dimensions
            const int32_t temp = bmpImage.infoHeader.bitmapWidth;
            bmpImage.infoHeader.bitmapWidth = bmpImage.infoHeader.bitmapHeight;
            bmpImage.infoHeader.bitmapHeight = temp;
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
            Image* blurred
                    = even_faster_image_blur(bmpImage.image, userInput->blur);
            if (blurred == NULL) {
                fprintf(stderr, "Blurring failed\n");
                break;
            }
            free_image(&(bmpImage.image));
            bmpImage.image = blurred;
        }

        if (userInput->dim) { // If dim mode enabled
            dim_effect(bmpImage.image, userInput->dim, userInput->dim,
                    userInput->dim); // FIX Update docs
        }

        if (userInput->melt) {
            if (melt(&bmpImage, userInput->meltOffset) == -1) {
                fprintf(stderr, "Melt failed\n"); // FIX
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
                fprintf(stderr, "Rotation failed\n");
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

[[nodiscard]] int filtr_col_check(uint8_t* setting, const char* arg)
{
    uint8_t colourBitVect = 0;

    for (const char* s = arg; *s != '\0'; s++) {

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
            *setting = 0;
            return -1;
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
        fprintf(stderr, fileTypeMessage, type);
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
