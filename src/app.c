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
	nanocad_init();

	char *line = "line x1.5;y30cm, ly1.2m";
	if (!parse_command(line)) {
		printf("Failed to parse the command.\n");
	}

	return 0;
}

/**
 * Prints a little "welcome" message.
 */
void print_welcome() {
	printf("nanoCAD wrapper v%s (engine v%s)\n\n", WRAPPER_VERSION, ENGINE_VERSION);
}

