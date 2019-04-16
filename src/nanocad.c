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

// Stored structures.
object_container objects;

// Command type definitions.
#define VALID_OBJECTS_SIZE 3
char valid_objects[VALID_OBJECTS_SIZE][COMMAND_MAX_SIZE] = { "line", "rect", "circle" };

// Function prototypes.
int parse_line(const char *line, char *command, char **arguments);
int is_obj_command(const char *command);


/**
 * Initializes the engine.
 */
void nanocad_init() {
	objects.count = 0;
}

/**
 * Creates a object in the object array.
 *
 * @param type Object type.
 * @param argc Number of arguments passed by the command.
 * @param argv Aguments passed by the command.
 */
void create_object(int type, char argc, char **argv) {
	// Create a new object.
	object_t obj;
	obj.type = (uint8_t)type;

	// Allocate the correct amount of memory for each type of object.
	switch (type) {
		case TYPE_LINE:
			obj.coord_count = 2;
			obj.coord = (coord_t *)malloc(sizeof(coord_t) * 2);
			obj.coord[0].x = 12;
			obj.coord[0].y = 34;
			obj.coord[1].x = 45;
			obj.coord[1].y = 67;
			break;
	}

	// Dynamically add the new object to the array.
	objects.list = realloc(objects.list,
			sizeof(object_t) * (objects.count + 1));
	objects.list[objects.count] = obj;
	objects.count++;
}

/**
 * Parses a command and executes it.
 *
 * @param  line A command line without the newline character at the end.
 * @return      TRUE if the parsing went fine.
 */
bool parse_command(const char *line) {
	int argc;
	char command[COMMAND_MAX_SIZE];
	char *argv[ARGUMENT_ARRAY_MAX_SIZE];

#ifdef DEBUG
	printf("> %s\n", line);
#endif

	if ((argc = parse_line(line, command, argv)) >= 0) {
		// Check which type of command this is.
		int type = -1;
		if ((type = is_obj_command(command)) > 0) {
			// Command will generate a object.
			create_object(type, argc, argv);
#ifdef DEBUG
			print_object_info(objects.list[objects.count - 1]);
#endif
		} else {
			// Not a known command.
			printf("Unknown command.\n");
			return false;
		}
#ifdef DEBUG
		printf("Command: %s - Arg. Count: %d\n", command, argc);
		for (int i = 0; i < argc; i++) {
			printf("Argument %d: %s\n", i, argv[i]);
		}
#endif

		return true;
	}

	return false;
}

/**
 * Checks if a command is of object type and returns the correct type for it.
 *
 * @param  command Command string to be checked.
 * @return         Object type if valid, otherwise -1.
 */
int is_obj_command(const char *command) {
	for (uint8_t i = 0; i < VALID_OBJECTS_SIZE; i++) {
		if (strcmp(command, valid_objects[i]) == 0) {
			return i + 1;
		}
	}

	return -1;
}

/**
 * Prints some debug information about a given object.
 *
 * @param object Object that you want to get information from.
 */
void print_object_info(const object_t object) {
	printf("Object Type: %d - %s\n", object.type, valid_objects[object.type - 1]);
	printf("Coordinates (%d total):\n", object.coord_count);

	for (uint8_t i = 0; i < object.coord_count; i++) {
		printf("    %d. (%ld, %ld)\n", i, object.coord[i].x, object.coord[i].y);
	}
}

/**
 * Parses a command line and separates each part.
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

/**
 * Gets a object from the objects array.
 *
 * @param  i Index of the object to be fetched.
 * @return   The requested object.
 */
object_t get_object(const size_t i) {
	return objects.list[i];
}

