/**
 * app.c
 * A simple wrapper for the nanoCAD engine.
 *
 * @author Nathan Campos <nathanpc@dreamintech.net>
 */

#include <stdio.h>
#include "nanocad.h"

// Constant definitions.
#define WRAPPER_VERSION "0.1a"

// Function prototypes.
void print_welcome();

/**
 * The program's main entry point.
 *
 * @param  argc Number of command-line arguments passed to the program.
 * @param  argv Array of command-line arguments passed to the program.
 * @return      Program return code.
 */
int main(int argc, char **argv) {
	// Show a little version message.
	print_welcome();

	char command[COMMAND_MAX_SIZE];
	char *args[ARGUMENT_ARRAY_MAX_SIZE];
	char *line = "line x1.5;y30cm, ly1.2m, testing";
	int _argc = -1;

	printf("> %s\n", line);
	_argc = parse_line(line, command, args);

	if (_argc >= 0) {
		printf("Command: %s - Arg. Count: %d\n", command, _argc);
		for (int i = 0; i < _argc; i++) {
			printf("Argument %d: %s\n", i, args[i]);
		}
	} else {
		printf("Couldn't parse the line.\n");
	}

	return 0;
}

/**
 * Prints a little "welcome" message.
 */
void print_welcome() {
	printf("nanoCAD wrapper v%s (engine v%s)\n\n", WRAPPER_VERSION, ENGINE_VERSION);
}

