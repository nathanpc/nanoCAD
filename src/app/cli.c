/**
 * app/cli.c
 * A simple command-line wrapper for the nanoCAD engine.
 *
 * @author Nathan Campos <nathanpc@dreamintech.net>
 */

#include <stdio.h>
#include <string.h>
#include "../engine/nanocad.h"
#include "../graphics/sdl_graphics.h"

// Constant definitions.
#define WRAPPER_VERSION "0.1a"

// Function prototypes.
void usage(char **argv);
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

	// Check for command line arguments.
	if (argc < 2) {
		// TODO: Present the command prompt.
		printf("Not implemented!\n");
		exit(EXIT_FAILURE);
	} else if (strcmp(argv[1], "-h") == 0) {
		usage(argv);
		exit(EXIT_SUCCESS);
	}

	// Parse the file.
	if (!nanocad_parse_file(argv[1])) {
		return EXIT_FAILURE;
	}
	
#ifndef MEMCHECK
	// Initialize the graphics.
	if (graphics_init(600, 450)) {
		graphics_eventloop();
	} else {
		graphics_clean();
		return EXIT_FAILURE;
	}
#endif

	// Clean up the mess left by the engine and return.
	nanocad_destroy();
	return 0;
}

/**
 * Prints a little "welcome" message.
 */
void print_welcome() {
	printf("nanoCAD wrapper v%s (engine v%s)\n\n", WRAPPER_VERSION,
		   ENGINE_VERSION);
}

/**
 * Prints the usage message.
 *
 * @param argv List of command line arguments.
 */
void usage(char **argv) {
	printf("Usage: %s [-h] [filename]\n", argv[0]);
	printf("\nArguments:\n");
	printf("    filename    A CAD file to be interpreted.\n");
	printf("\nFlags:\n");
	printf("    -h    Shows this message.\n");
}

