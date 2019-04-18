/**
 * app.c
 * A simple wrapper for the nanoCAD engine.
 *
 * @author Nathan Campos <nathanpc@dreamintech.net>
 */

#include <stdio.h>
#include <string.h>
#include "nanocad.h"

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

	if (argc < 2) {
		// TODO: Present the command prompt.
		printf("Not implemented!\n");
		exit(EXIT_FAILURE);
	} else if (strcmp(argv[1], "-h") == 0) {
		usage(argv);
		exit(EXIT_SUCCESS);
	}

	// Open the CAD file for parsing.
	char *filename = argv[1];
	FILE *fp = fopen(filename, "r");
	if (fp == NULL) {
		printf("Couldn't open the CAD file: %s\n", filename);
		exit(EXIT_FAILURE);
	}

	// Go through each line.
	char *line;
	size_t len = 0;
	ssize_t read;
	unsigned int linenum = 1;
	while ((read = getline(&line, &len, fp)) != -1) {
		// Remove the trailling newline.
		if (line[read - 1] == '\n') {
			line[read - 1] = '\0';
		}

#ifdef DEBUG
		printf("Line %d: %s\n", linenum, line);
#endif

		// Parse lines.
		if (!parse_command(line)) {
			printf("Failed to parse line %d: %s\n", linenum, line);
		}

		linenum++;
	}

	// Some clean-up.
	fclose(fp);
	free(line);

	return 0;
}

/**
 * Prints a little "welcome" message.
 */
void print_welcome() {
	printf("nanoCAD wrapper v%s (engine v%s)\n\n", WRAPPER_VERSION, ENGINE_VERSION);
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

