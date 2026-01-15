// Included Libraries
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "commands.h"
#include "errors.h"

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
          "  -h, --hue <R, G, B>         - Increase/decrease channel intensity "
          "by a constant (-255 <-> 255)\n"
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

/* any_empty_args()
 * ----------------
 * Searches an array of strings for empty string arguments.
 *
 * argc: Number of string in the string array.
 * argv: Array of strings.
 *
 * Returns: true if empty args found, false otherwise.
 */
static inline bool any_empty_args(const int argc, char** argv)
{
    for (int i = 0; i < argc; i++) {
        if (argv[i][0] == '\0') {
            return true;
        }
    }

    return false;
}

static inline void help_message(void)
{
    fputs(helpMessage, stdout);
}

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

    if (!(strcmp(argv[1], "help"))) {
        if (argc == 2) {
            help_message();

        } else if (argc == 3) {
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

    int status = parse_user_commands(argc, argv);
    if (status != EXIT_SUCCESS) {
        exit(status);
    }

    status = handle_commands();
    return status;
}
