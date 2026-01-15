#ifndef COMMANDS_H
#define COMMANDS_H

#define fileTypeMessage "Input must be a \'%s\' file.\n"
#define invalidCmdMessage                                                      \
    "signals: \'%s\' is not a valid command. See \'signals help\'\n"

#define userHelpPrompt "See \'signals help\'.\n"
#define nonUniquePathsMessage                                                  \
    "signals: input and combine file paths must be unique!\n"
#define fileType ".bmp"
#define invalidVal "signals: invalid value \'%s\'\n"

// Error messages
#define unexpectedArgMessage "Got \'%s\', expected \'%s\'\n"
#define gotStrMessage "    Got \'%s\'.\n"
#define invalidFilterColourMessage                                             \
    "signals: filter colour/s \'%s\' are invalid, must be RGB characters.\n"

int parse_user_commands(const int argc, char** argv);
int handle_commands(void);
int command_list(const char* command);

#endif
