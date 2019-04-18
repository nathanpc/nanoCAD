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
#define PARSING_START     0
#define PARSING_COMMAND   1
#define PARSING_ARGUMENTS 2
#define PARSING_COORDX    3
#define PARSING_COORDY    4
#define PARSING_WIDTH     6
#define PARSING_HEIGHT    7
#define PARSING_NUMBER    8
#define PARSING_UNIT      9

// Operation type definitions.
#define OPERATION_WIDTH  'w'
#define OPERATION_HEIGHT 'h'

// Stored structures.
object_container objects;

// Command type definitions.
#define VALID_OBJECTS_SIZE 3
char valid_objects[VALID_OBJECTS_SIZE][COMMAND_MAX_SIZE] = { "line", "rect", "circle" };

// Function prototypes.
int parse_line(const char *line, char *command, char **arguments);
int is_obj_command(const char *command);
long to_base_unit(const char *str);
void create_object(int type, char argc, char **argv);
void nanocad_init();

/**
 * Initializes the engine.
 */
void nanocad_init() {
	objects.count = 0;
}

void calc_coordinate(const char oper, const coord_t base, coord_t *coord) {
	switch (oper) {
		case OPERATION_WIDTH:
			// Using coord->x to store the width already.
			coord->x += base.x;
			coord->y = base.y;
			break;
		case OPERATION_HEIGHT:
			// Using coord->y to store the height already.
			coord->x = base.x;
			coord->y += base.y;
			break;
		default:
			printf("Invalid coordinate operation: %c.\n", oper);
			exit(1);
	}
}

/**
 * Parses a coordinate argument into a internal coordinate structure.
 *
 * @param coord Pointer to the output of the function.
 * @param arg   The argument raw string to be parsed.
 * @param base  A base coordinate for "length calculations", can be NULL if
 *              there isn't one.
 */
void parse_coordinates(coord_t *coord, const char *arg, const coord_t *base) {
	uint8_t stage = PARSING_START;
	uint8_t cur_pos = 0;
	char operation = '\0';
	char coord_x[ARGUMENT_MAX_SIZE];
	char coord_y[ARGUMENT_MAX_SIZE];
	coord_x[0] = '0';
	coord_y[0] = '0';
	coord_x[1] = '\0';
	coord_y[1] = '\0';

	// Iterate over the argument string until we hit the NULL terminator.
	while (*arg != '\0') {
		// Get the current character.
		char c = *arg++;

		switch (stage) {
			case PARSING_START:
				if (c == 'x') {
					cur_pos = 0;
					stage = PARSING_COORDX;
				} else if ((c == 'w') || (c == 'h')) {
					// TODO: Make sure we can parse something like w10cm;h25cm.
					cur_pos = 0;
					operation = c;

					if (c == 'w') {
						stage = PARSING_WIDTH;
					} else if (c == 'h') {
						stage = PARSING_HEIGHT;
					}
				} else {
					printf("Unknown first coordinate letter: %c.\n", c);
					exit(1);
				}
				break;
			case PARSING_COORDX:
				if (c == ';') {
					cur_pos = 0;
					stage = PARSING_ARGUMENTS;
				} else {
					coord_x[cur_pos++] = c;
					coord_x[cur_pos] = '\0';
				}
				break;
			case PARSING_ARGUMENTS:
				if (c == 'y') {
					cur_pos = 0;
					stage = PARSING_COORDY;
				} else {
					printf("Unknown next argument start for coordinate: %c.\n", c);
					exit(1);
				}
				break;
			case PARSING_COORDY:
				coord_y[cur_pos++] = c;
				coord_y[cur_pos] = '\0';
				break;
			case PARSING_WIDTH:
				coord_x[cur_pos++] = c;
				coord_x[cur_pos] = '\0';
				break;
			case PARSING_HEIGHT:
				coord_y[cur_pos++] = c;
				coord_y[cur_pos] = '\0';
				break;
		}
	}

	// Convert coordinates.
	coord->x = to_base_unit(coord_x);
	coord->y = to_base_unit(coord_y);

	// Looks like we'll need to calculate some stuff.
	if ((base != NULL) && (operation != '\0')) {
		calc_coordinate(operation, *base, coord);
	}
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
			parse_coordinates(&obj.coord[0], argv[0], NULL);
			parse_coordinates(&obj.coord[1], argv[1], &obj.coord[0]);
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
#ifdef DEBUG
		printf("Command: %s - Arg. Count: %d\n", command, argc);
		for (int i = 0; i < argc; i++) {
			printf("Argument %d: %s\n", i, argv[i]);
		}
#endif
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

		return true;
	}

	return false;
}

/**
 * Converts a raw string to a long in the base unit.
 *
 * @param  str Raw string in a coordinate form.
 * @return     Number in the base unit.
 */
long to_base_unit(const char *str) {
	long num = 0;
	char unit[3];
	char strnum[ARGUMENT_MAX_SIZE];
	uint8_t stage = PARSING_NUMBER;
	uint8_t cur_pos = 0;
	unit[0] = '\0';

	// Iterate over the coordinate string until we hit the NULL terminator.
	while (*str != '\0') {
		// Get the current character.
		char c = *str++;

		switch (stage) {
			case PARSING_NUMBER:
				if (((c >= 0x30) && (c <= 0x39)) || 
						((c >= 0x2B) && (c <= 0x2E))) {
					// Character is a number.
					strnum[cur_pos++] = c;
					strnum[cur_pos] = '\0';
				} else if ((c >= 0x61) && (c <= 0x7A)) {
					// Character is a letter.
					cur_pos = 0;
					stage = PARSING_UNIT;
					unit[cur_pos++] = c;
					unit[cur_pos] = '\0';
				} else {
					// Invalid character.
					printf("Invalid character found while trying to parse a "
							"number: %c.\n", c);
					exit(1);
				}
				break;
			case PARSING_UNIT:
				if ((c >= 0x61) && (c <= 0x7A)) {
					// Character is a letter.
					unit[cur_pos++] = c;
					unit[cur_pos] = '\0';
				} else {
					// Invalid character.
					printf("Invalid character found while trying to parse a "
							"unit: %c.\n", c);
					exit(1);
				}
				break;
		}
	}

	// Parse number from string and convert it.
	double orig = atof(strnum);
	if (unit[0] == '\0') {
		// Already at the base unit.
		num = (long)orig;
	} else if (!strcmp(unit, "m")) {
		// Meters.
		num = (long)(orig * 1000);
	} else if (!strcmp(unit, "cm")) {
		// Centimeters.
		num = (long)(orig * 10);
	} else if (!strcmp(unit, "mm")) {
		// Millimeters.
		num = (long)orig;
	} else {
		// Invalid unit.
		printf("Invalid unit: %s\n", unit);
		exit(1);
	}

#ifdef DEBUG
	printf("Number: %s - Unit: %s - Double: %f - Final: %ld\n", strnum, unit, orig, num);
#endif

	return num;
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
						cur_arg[cur_cpos++] = c;
						cur_arg[cur_cpos] = '\0';
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

