/**
 * nanocad.c
 * A tiny CAD engine with a super simple syntax designed to be easily embeddable.
 *
 * @author Nathan Campos <nathanpc@dreamintech.net>
 */

#include "nanocad.h"

#include <stdio.h>
#include <string.h>

// Line parsing stage definitions.
#define PARSING_COMMAND   0
#define PARSING_ARGUMENTS 1

/**
 * Parses a command line.
 *
 * @param  line      The command line without the newline character at the end.
 * @param  command   Pointer to a string that will contain the command after
 *                   parsing.
 * @param  arguments Array of strings that will contain the arguemnts.
 * @return           Number of arguments found for the command or -1 if there was
 *                   an error while parsing.
 */
int parse_line(const char *line, char *command, char **arguments) {
	uint8_t stage = PARSING_COMMAND;
	uint16_t cur_cpos = 0;
	int argc = -1;
	char cur_arg[ARGUMENT_MAX_SIZE];
	
	// Reset the command string.
	command[0] = '\0';

	// Iterate over the line string until we hit the NULL terminator.
	while (*line != '\0') {
		// Get the current character.
		char c = *line++;

		// Treat each stage of parsing differently.
		switch (stage) {
			case PARSING_COMMAND:
				if (c == ' ') {
					// Space found, so the command part has ended.
					command[cur_cpos] = '\0';
					cur_cpos = 0;
					argc = 0;
					stage = PARSING_ARGUMENTS;
				} else {
					if ((cur_cpos + 1) < COMMAND_MAX_SIZE) {
						// Append characters to the command string.
						command[cur_cpos] = c;
						cur_cpos++;
					} else {
						printf("ERROR: Command maximum character limit "
								"exceeded.\n");
						return -1;
					}
				}
				break;
			case PARSING_ARGUMENTS:
				if (c == ',') {
					// Comma found, so the argument has ended.
					arguments[argc - 1] = strdup(cur_arg);
					cur_cpos = 0;
					cur_arg[0] = '\0';

					if (argc == ARGUMENT_ARRAY_MAX_SIZE) {
						printf("Maximum number of arguments exceeded.\n");
						return -1;
					}
				} else if ((c == ' ') && (cur_cpos == 0)) {
					// Ignoring the space after commas.
				} else {
					// Increment the argument counter if at start of an argument.
					if (cur_cpos == 0) {
						argc++;
					}

					if ((cur_cpos + 1) < ARGUMENT_MAX_SIZE) {
						cur_arg[cur_cpos] = c;
						cur_arg[cur_cpos + 1] = '\0';
						cur_cpos++;
					} else {
						printf("Maximum argument character size exceeded on "
								"argument number %d.\n", argc);
						return -1;
					}
				}
				break;
			default:
				printf("ERROR: Unknown line parsing state. This shouldn't "
						"happen.\n");
				return -1;
		}
	}

	// Store the last argument parsed.
	if (argc > 0) {
		arguments[argc - 1] = strdup(cur_arg);
	}

	return argc;
}

