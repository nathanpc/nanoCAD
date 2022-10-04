/**
 * engine/nanocad.c
 * A tiny CAD engine with a super simple syntax designed to be easily embeddable.
 *
 * @author Nathan Campos <nathanpc@dreamintech.net>
 */

#include "nanocad.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

// Line parsing stage definitions.
#define PARSING_START      0
#define PARSING_COMMAND    1
#define PARSING_ARGUMENTS  2
#define PARSING_COORDX     3
#define PARSING_COORDY     4
#define PARSING_WIDTH      6
#define PARSING_HEIGHT     7
#define PARSING_NUMBER     8
#define PARSING_UNIT       9
#define PARSING_SET_OBJVAR 10

// Operation type definitions.
#define OPERATION_WIDTH  'w'
#define OPERATION_HEIGHT 'h'

// Variable type definitions.
#define VARIABLE_FIXED  '$'
#define VARIABLE_COORD  '@'
#define VARIABLE_OBJECT '&'

// Stored structures.
object_container    objects;
variable_container  variables;
history_container   history;
layer_container     layers;
dimension_container dimensions;
variable_t          last_object;

// Command type definitions.
#define VALID_OBJECTS_SIZE 3
char valid_objects[VALID_OBJECTS_SIZE][COMMAND_MAX_SIZE] = { 
	"line",
	"rect",
	"circle"
};

// Commands that shouldn't have variables substituted in their arguments.
#define NOVARSUBS_COMMAND_SIZE 1
char nosubstitute_commands[NOVARSUBS_COMMAND_SIZE][ARGUMENT_MAX_SIZE] = {
	"inspect"
};

/**
 * Internal functions.
 */
// Various utilities.
void free_array(void **arr, const size_t len);
void chomp(char *str);
int is_obj_command(const char *command);
bool is_no_substitute_command(const char *command);
long to_base_unit(const char *str);
uint8_t hex_to_dec(const char *hex);

// History.
void add_history_line(const char *line);

// Variables.
variable_t* get_variable(const char *name);
void variable_strval(const char *name, const uint8_t coord_index,
					 char strval[ARGUMENT_MAX_SIZE]);
void set_variable(const char *name, const char *value);
int substitute_variables(const char *command, char arg[ARGUMENT_MAX_SIZE]);

// Layers.
uint8_t parse_layer_num(const char *arg);
layer_t* get_layer(const uint8_t num);
void set_layer(const uint8_t num, const char *name, const char *color);

// Parsing.
void parse_rgb_color(const char *str, rgba_color_t *color);
int parse_line(const char *line, char *command, char **arguments);
void parse_coordinates(coord_t *coord, const char *arg, const coord_t *base);
bool parse_command(const char *line);
bool parse_file(const char *filename);

// Coordinates.
void calc_coordinate(const char oper, const coord_t base, coord_t *coord);

// Dimensions.
bool create_dimension(const int argc, char **argv, const bool is_offset);

// Objects.
void create_object(const int type, const int argc, char **argv);
object_t get_object(const size_t i);

// Debug.
bool inspect(char *thing);


/**
 * Initializes the engine.
 */
void nanocad_init() {
	// Initialize the container counts.
	objects.count = 0;
	variables.count = 0;
	history.count = 0;
	layers.count = 0;
	dimensions.count = 0;
	
	// Initialize last object.
	last_object.type = '&';
	last_object.name = NULL;
	last_object.value = NULL;
	
	// Create the default 0 layer.
	set_layer(0, "Default", "f9f9f9");
}

/**
 * Destroys everything related to the engine and frees the memory properly.
 */
void nanocad_destroy() {
	// Free all of the history lines.
	free_array((void**)history.lines, history.count);

	// Free all of the variables.
	for (size_t i = 0; i < variables.count; i++) {
		free(variables.list[i].name);
		free(variables.list[i].value);
	}

	// Free all of the objects.
	for (size_t i = 0; i < objects.count; i++) {
		free(objects.list[i].coord);
	}
	
	// Free all of the layers.
	for (size_t i = 0; i < layers.count; i++) {
		free(layers.list[i].name);
	}
	
	// Free up the last object variable.
	free(last_object.name);
	free(last_object.value);
	
	// Free all of the containers.
	free(variables.list);
	free(objects.list);
	free(history.lines);
	free(layers.list);
	free(dimensions.list);
}

/**
 * Parses a command and executes it.
 *
 * @param  line A command line without the newline character at the end.
 * @return      TRUE if the parsing went fine.
 */
bool nanocad_parse_command(const char *line) {
	return parse_command(line);
}

/**
 * Parses a nanoCAD formatted file.
 *
 * @param  filename Path to the file to be parsed.
 * @return          TRUE if everything went OK.
 */
bool nanocad_parse_file(const char *filename) {
	return parse_file(filename);
}

/**
 * Gets a layer object based on its layer number.
 * 
 * @param  num Layer number.
 * @return     Layer object pointer or NULL if a layer wasn't found.
 */
layer_t* nanocad_get_layer(const uint8_t num) {
	return get_layer(num);
}

/**
 * Gets a object from the objects array.
 *
 * @param  i Index of the object to be fetched.
 * @return   The requested object.
 */
object_t nanocad_get_object(const size_t i) {
	return get_object(i);
}

/**
 * Retrieves the internal object container for external use.
 *
 * @param container Pointer to the internal object container.
 */
void nanocad_get_object_container(object_container *container) {
	*container = objects;
}

/**
 * Retrieves the internal dimension container for external use.
 * 
 * @param container Pointer to the internal dimension container.
 */
void nanocad_get_dimension_container(dimension_container *container) {
	*container = dimensions;
}

/**
 * Prints some debug information about a variable or layer.
 * Warning: This function alters the contents of "*thing".
 * 
 * @param  thing Thing to be inspected in string form.
 * @return       TRUE if the inspecting was successful.
 */
bool nanocad_inspect(char *thing) {
	return inspect(thing);
}

/**
 * Adds a new layer to the layer container.
 * 
 * @param num   Layer number.
 * @param name  Layer name.
 * @param color RGB hexadecimal color string.
 */
void set_layer(const uint8_t num, const char *name, const char *color) {
	layer_t layer;
	
	// Check if the user is trying to mess with the 0 layer.
	if ((num == 0) && (layers.count > 0)) {
		printf("Can't alter any parameters of the 0 layer. The 0 layer is "
			   "read-only.\n");
		exit(EXIT_FAILURE);
	}
	
	// TODO: Check if layer number already exists.

	// Populate the layer object.
	layer.num = num;
	layer.name = strdup(name);
	parse_rgb_color(color, &layer.color);

	// Dynamically add the new layer to the array.
	layers.list = realloc(layers.list, sizeof(layer_t) * (layers.count + 1));
	layers.list[layers.count++] = layer;
	
#ifdef DEBUG
	print_layer_info(*get_layer(num));
#endif
}

/**
 * Gets a layer object based on its layer number.
 * 
 * @param  num Layer number.
 * @return     Layer object pointer or NULL if a layer wasn't found.
 */
layer_t* get_layer(const uint8_t num) {
	for (uint8_t i = 0; i < layers.count; i++) {
		if (layers.list[i].num == num) {
			return &layers.list[i];
		}
	}
	
	return NULL;
}

/**
 * Parses a layer number from a argument string.
 * 
 * @param  arg Argument string in the form of "l<num>".
 * @return     Layer number.
 */
uint8_t parse_layer_num(const char *arg) {
	char str_num[4];
	
	// Parse the number from the string.
	for (uint8_t i = 0; i < 4; i++) {
		str_num[i] = arg[i + 1];
		
		// Check if we have reached a premature end of string.
		if (arg[i + 1] == '\0') {
			break;
		}
	}
		
	return (uint8_t)atoi(str_num);
}

/**
 * Parses a RGB(A) color string and stores it into a color structure pointer.
 * 
 * @param str   RGB(A) color string.
 * @param color Pointer to a color structure.
 */
void parse_rgb_color(const char *str, rgba_color_t *color) {
	// Separate each string color.
	char red[3] = { str[0], str[1], '\0' };
	char green[3] = { str[2], str[3], '\0' };
	char blue[3] = { str[4], str[5], '\0' };
	
	// Convert from hexadecimal to decimal.
	color->alpha = 255;
	color->r = hex_to_dec(red);
	color->g = hex_to_dec(green);
	color->b = hex_to_dec(blue);
	
#ifdef DEBUG
	printf("Color string: %s - R(0x%s) G(0x%s) B(0x%s) - RGBA(%d, %d, %d, %d)\n",
		   str, red, green, blue, color->r, color->g, color->b, color->alpha);
#endif
}

/**
 * Prints some debug information about the layer.
 * 
 * @param layer Layer object to inspect.
 */
void print_layer_info(const layer_t layer) {
	printf("Layer %d '%s':\n", layer.num, layer.name);
	printf("    Color: RGB(%d, %d, %d)\n", layer.color.r, layer.color.g,
		   layer.color.b);
	printf("    Alpha: %d\n", layer.color.alpha);
}

/**
 * Sets a internal variable.
 * 
 * @param name  Variable name.
 * @param value Variable value.
 */
void set_variable(const char *name, const char *value) {
	size_t obj_index;
	variable_t var;
	var.type = *name++;
	var.name = strdup(name);
	
	// Check if the variable already exists.
	variable_t *old_var = get_variable(name);
	if (old_var != NULL) {
		// Ignore find if it is the last object variable.
		if (old_var->name[0] != '^') {
			// TODO: Check if already exists, if so, override.
			printf("Variable '%s' already exists. Can't set a new value\n", name);
			printf("This will be implemented in the future.\n");
			exit(EXIT_FAILURE);
		}
	}
	
	// Parse variable value according to type.
	switch (var.type) {
	case VARIABLE_FIXED:
		// Fixed value.
		var.value = malloc(sizeof(double*));
		*((double*)var.value) = atof(value);
		break;
	case VARIABLE_COORD:
		// Coordinate.
		var.value = malloc(sizeof(coord_t*));
		parse_coordinates((coord_t*)var.value, value, NULL);
		break;
	case VARIABLE_OBJECT:
		// Object.
		if (sscanf(value, "%zu", &obj_index) == 1) {
			if ((name[0] == '^') && (name[1] == '\0')) {
				// Last object variable setting.
				last_object.value = malloc(sizeof(objects.list[obj_index]));
				last_object.value = &objects.list[obj_index];
			} else {
				// Normal object variable setting.
				var.value = malloc(sizeof(objects.list[obj_index]));
				var.value = &objects.list[obj_index];
			}
		} else {
			printf("Couldn't parse object index when assigning object to "
				   "variable.\n");
			exit(EXIT_FAILURE);
		}
		break;
	default:
		printf("Invalid variable type '%c' in %s\n", var.type, var.name);
		exit(EXIT_FAILURE);
	}

	// Check if it is a last object type variable set.
	if ((name[0] == '^') && (name[1] == '\0')) {
		if (last_object.name == NULL) {
			last_object.name = strdup("^");
		}
		
		return;
	}
	
	// Dynamically add the new variable to the array.
	variables.list = realloc(variables.list,
							 sizeof(variable_t) * (variables.count + 1));
	variables.list[variables.count++] = var;
}

/**
 * Gets a variable value by its name.
 * 
 * @param  name Variable name to be searched for.
 * @return      Pointer to the variable structure or NULL if it wasn't found.
 */
variable_t* get_variable(const char *name) {
	// Check if we are looking for the last object variable.
	if ((name[0] == '^') && (name[1] == '\0')) {
		// Check if we have a valid last object.
		if (last_object.name != NULL) {
			return &last_object;
		}
		
		return NULL;
	}
	
	// Search through the variables.
	for (size_t i = 0; i < variables.count; i++) {
		// If the names are the same then return a pointer to the variable.
		if (strcmp(name, variables.list[i].name) == 0) {
			return &variables.list[i];
		}
	}

	return NULL;
}

/**
 * Gets a string representation of the variable value to be substituted into
 * a command.
 * 
 * @param name        Variable name to be searched for.
 * @param coord_index Coordinate index for object variables.
 * @param strval      String representation of the variable value.
 */
void variable_strval(const char *name, const uint8_t coord_index,
					 char strval[ARGUMENT_MAX_SIZE]) {
	// Check if there is any variable with this name.
	variable_t *var = get_variable(name);
	if (var == NULL) {
		printf("Variable '%s' not found\n", name);
		exit(EXIT_FAILURE);
	}

	// Output the correct string depending on the variable type.
	switch (var->type) {
	case VARIABLE_FIXED:
		// Fixed Value
		snprintf(strval, ARGUMENT_MAX_SIZE, "%f", *((double*)var->value));
		break;
	case VARIABLE_COORD:
		// Coordinate
		snprintf(strval, ARGUMENT_MAX_SIZE, "x%ld;y%ld",
				 ((coord_t*)var->value)->x, ((coord_t*)var->value)->y);
		break;
	case VARIABLE_OBJECT:
		// Object
		if (coord_index < ((object_t*)var->value)->coord_count) {
			// Requested coordinate found.
			snprintf(strval, ARGUMENT_MAX_SIZE, "x%ld;y%ld",
					 ((object_t*)var->value)->coord[coord_index].x,
					 ((object_t*)var->value)->coord[coord_index].y);
		} else {
			// Wrong coordinate index.
			printf("Variable '&%s[%d]' index is greater than the maximum "
				   "allowed for this type of object: %d\n", name, coord_index,
				   ((object_t*)var->value)->coord_count);
			exit(EXIT_FAILURE);
		}
		break;
	default:
		printf("Invalid variable type '%c' in variable '%s'\n",
			   var->type, var->name);
		exit(EXIT_FAILURE);
	}
}

/**
 * Prints some debug information about a variable.
 * 
 * @param var Variable to be debuged.
 */
void print_variable_info(const variable_t var) {
	printf("Variable Type: %c - ", var.type);

	// Print human-readable type.
	switch (var.type) {
	case VARIABLE_FIXED:
		printf("Fixed Value\n");
		break;
	case VARIABLE_COORD:
		printf("Coordinate\n");
		break;
	case VARIABLE_OBJECT:
		printf("Object\n");
		break;
	default:
		printf("UNKNOWN\n");
	}

	printf("Name: %s - Value: ", var.name);
	
	// Print value according to type.
	char strval[ARGUMENT_MAX_SIZE];
	object_t *obj;
	switch (var.type) {
	case VARIABLE_FIXED:
		variable_strval(var.name, 0, strval);
		printf("%f - String: %s\n", *((double*)var.value), strval);
		break;
	case VARIABLE_COORD:
		variable_strval(var.name, 0, strval);
		printf("(%lu, %lu) - String: %s\n", ((coord_t*)var.value)->x,
			   ((coord_t*)var.value)->y, strval);
		break;
	case VARIABLE_OBJECT:
		obj = (object_t*)var.value;
		print_object_info(*obj);

		printf("String Representation:\n");
		for (uint8_t i = 0; i < obj->coord_count; i++) {
			variable_strval(var.name, i, strval);
			printf("&%s[%d] -> %s\n", var.name, i, strval);
		}
		break;
	default:
		printf("UNKNOWN\n");
	}
}

/**
 * Calculates the coordinates based on a operation.
 *
 * @param oper  Operation type.
 * @param base  Base coordinate.
 * @param coord Operation specific coordinate.
 */
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
		exit(EXIT_FAILURE);
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
				exit(EXIT_FAILURE);
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
				exit(EXIT_FAILURE);
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
 * Creates a new dimension and puts it into the dimension container.
 * 
 * @param  argc      Number of arguments passed by the command.
 * @param  argv      Aguments passed by the command.
 * @param  is_offset Was the command called as a offset dimension?
 * @return           TRUE if everything went fine.
 */
bool create_dimension(const int argc, char **argv, const bool is_offset) {
	dimension_t dimen;
	dimen.layer_num = 0;
	
	// Check argument limits.
	if ((argc < 4) && (argc > 5)) {
		return false;
	}
	
	// Check for optional arguments.
	if (argv[argc - 1][0] == 'l') {
		// Setting a layer.
		dimen.layer_num = parse_layer_num(argv[argc - 1]);
	}
	
	// Parse the measured coordinates.
	parse_coordinates(&dimen.start, argv[0], NULL);
	parse_coordinates(&dimen.end, argv[1], NULL);
	
	// Parse the dimension line coordinates.
	if (is_offset) {
		// Offset the dimension line.
		coord_t ostart;
		coord_t oend;
		coord_t delta;
		
		// Get offset distance.
		long offset = to_base_unit(argv[3]);
		
		// Make sure all dimension lines are going from left to right
		// and top to bottom.
		if (dimen.start.y == dimen.end.y) {
			// Straight horizontal lines.
			if (dimen.start.x < dimen.end.x) {
				// Line going left to right.
				ostart.x = dimen.start.x;
				ostart.y = dimen.start.y;
				oend.x = dimen.end.x;
				oend.y = dimen.end.y;
			} else {
				// Line going right to left.
				ostart.x = dimen.end.x;
				ostart.y = dimen.end.y;
				oend.x = dimen.start.x;
				oend.y = dimen.start.y;
			}
		} else if (dimen.start.x == dimen.end.x) {
			// Straight vertical lines.
			if (dimen.start.y > dimen.end.y) {
				// Line going top to bottom.
				ostart.x = dimen.start.x;
				ostart.y = dimen.start.y;
				oend.x = dimen.end.x;
				oend.y = dimen.end.y;
			} else {
				// Line going bottom to top.
				ostart.x = dimen.end.x;
				ostart.y = dimen.end.y;
				oend.x = dimen.start.x;
				oend.y = dimen.start.y;
			}
		} else if (dimen.start.y > dimen.end.y) {
			// Non-straight lines going top to bottom.
			if (dimen.start.x < dimen.end.x) {
				// Line going left to right.
				ostart.x = dimen.start.x;
				ostart.y = dimen.start.y;
				oend.x = dimen.end.x;
				oend.y = dimen.end.y;
			} else {
				// Line going right to left.
				ostart.x = dimen.end.x;
				ostart.y = dimen.end.y;
				oend.x = dimen.start.x;
				oend.y = dimen.start.y;
			}
		} else if (dimen.start.y < dimen.end.y) {
			// Non-straight lines going bottom to top.
			if (dimen.start.x < dimen.end.x) {
				// Line going left to right.
				ostart.x = dimen.start.x;
				ostart.y = dimen.start.y;
				oend.x = dimen.end.x;
				oend.y = dimen.end.y;
			} else {
				// Line going right to left.
				ostart.x = dimen.end.x;
				ostart.y = dimen.end.y;
				oend.x = dimen.start.x;
				oend.y = dimen.start.y;
			}
		}
		
		// Calculate the parallel line parameters.
		delta.x = ostart.x - oend.x;
		delta.y = ostart.y - oend.y;
		long dist = (long)roundl(sqrtl((delta.x * delta.x) +
									   (delta.y * delta.y)));
		delta.x = (long)nearbyintl((long double)delta.x / dist);
		delta.y = (long)nearbyintl((long double)delta.y / dist);
		
		// Calculate the dimension line position.
		if (argv[2][0] == 'u') {
			// Above measured line.
			dimen.line_start.x = dimen.start.x;
			dimen.line_start.y = dimen.start.y - (offset * delta.x);
			dimen.line_end.x = dimen.end.x;
			dimen.line_end.y = dimen.end.y - (offset * delta.x);
			
			// Diagonal dimension.
			if (argv[2][1] == 'l') {
				dimen.line_start.x = dimen.start.x + (offset * delta.y);
				dimen.line_end.x = dimen.end.x + (offset * delta.y);
			} else if (argv[2][1] == 'r') {
				dimen.line_start.x = dimen.start.x + (offset * delta.y);
				dimen.line_end.x = dimen.end.x + (offset * delta.y);
			}
		} else if (argv[2][0] == 'd') {
			// Below measured line.
			dimen.line_start.x = dimen.start.x;
			dimen.line_start.y = dimen.start.y + (offset * delta.x);
			dimen.line_end.x = dimen.end.x;
			dimen.line_end.y = dimen.end.y + (offset * delta.x);
			
			// Diagonal dimension.
			if (argv[2][1] == 'l') {
				dimen.line_start.x = dimen.start.x - (offset * delta.y);
				dimen.line_end.x = dimen.end.x - (offset * delta.y);
			} else if (argv[2][1] == 'r') {
				// TODO: Fix this.
				dimen.line_start.x = dimen.start.x - (offset * delta.y);
				dimen.line_end.x = dimen.end.x - (offset * delta.y);
			}
		} else if (argv[2][0] == 'r') {
			// Right of measured line.
			dimen.line_start.x = dimen.start.x + (offset * delta.y);
			dimen.line_start.y = dimen.start.y;
			dimen.line_end.x = dimen.end.x + (offset * delta.y);
			dimen.line_end.y = dimen.end.y;
		} else if (argv[2][0] == 'l') {
			// Left of measured line.
			dimen.line_start.x = dimen.start.x - (offset * delta.y);
			dimen.line_start.y = dimen.start.y;
			dimen.line_end.x = dimen.end.x - (offset * delta.y);
			dimen.line_end.y = dimen.end.y;
		} else {
			printf("Unknown dimension offset direction: '%s'\n", argv[2]);
			return false;
		}
	} else {
		// Use the manually inserted coordinates.
		parse_coordinates(&dimen.line_start, argv[2], NULL);
		parse_coordinates(&dimen.line_end, argv[3], NULL);
	}
	
	// Dynamically add the new dimension to the array.
	dimensions.list = realloc(dimensions.list,
							  sizeof(dimension_t) * (dimensions.count + 1));
	dimensions.list[dimensions.count++] = dimen;

	return true;
}

/**
 * Creates a object in the object array.
 *
 * @param type Object type.
 * @param argc Number of arguments passed by the command.
 * @param argv Aguments passed by the command.
 */
void create_object(const int type, const int argc, char **argv) {
	// Create a new object.
	object_t obj;
	obj.type = (uint8_t)type;
	obj.layer_num = 0;
	
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
	objects.list[objects.count++] = obj;
	
	// Pass the object index as a string to the variable setting function.
	char str_idx[VARIABLE_MAX_SIZE];
	snprintf(str_idx, VARIABLE_MAX_SIZE, "%lu", objects.count - 1);
	set_variable("&^", str_idx);  // Set the last object variable.
	
	// TODO: Find a way to check for variable and layers and optional arguments.
	
	// Check if we need to store this object into a variable.
	int last_index = argc - 1;
	if (argv[last_index][0] == '&') {
		set_variable(argv[last_index], str_idx);
		last_index--;  // Last index won't be the variable name anymore.
		
#ifdef DEBUG
		print_variable_info(variables.list[variables.count - 1]);
#endif
	}
	
	// Check for optional arguments.
	if (argv[last_index][0] == 'l') {
		// We have a layer setting.
		objects.list[objects.count - 1].layer_num =
			parse_layer_num(argv[last_index]);
	}
}

/**
 * Substitute the variables for their values in a argument.
 * 
 * @param  command Command that had the argument passed to this function.
 * @param  arg     Argument that will be changed.
 * @return         Number of variables substituted.
 */
int substitute_variables(const char *command, char arg[ARGUMENT_MAX_SIZE]) {
	int sub_count = 0;
	uint8_t char_count = 0;
	uint8_t name_ccount = 0;
	int begin = -1;
	int end = -1;
	uint8_t index = 0;
	char *orig = strdup(arg);
	char var_name[VARIABLE_MAX_SIZE];
	var_name[0] = '\0';
	
	// Check if it is a no substitution command.
	if (is_no_substitute_command(command)) {
		return 0;
	}
	
	// Check for commands that shouldn't have variables substituted.
	if (strcmp("set", command) == 0) {
		return 0;
	}
	
	// Iterate over the argument until we hit the NULL terminator.
	for (char_count = 0; orig[char_count] != '\0'; char_count++) {
		// Get the current character.
		char c = orig[char_count];

		if (begin < 0) {
			// Check for a variable beginning.
			if ((c == '$') || (c == '@') || (c == '&')) {
				begin = char_count;
			}
		} else {
			// Parsing a variable name.
			if (((c >= 'a') && (c <= 'z')) ||
				((c >= 'A') && (c <= 'Z')) ||
				((c >= '0') && (c <= '9')) || (c == '^')) {
				// Variable name.
				var_name[name_ccount++] = c;
				var_name[name_ccount] = '\0';
			} else if (c == '[') {
				// Get index and finish this.
				char_count++;
				c = orig[char_count];  // Get index character.
				index = c - '0';       // Convert character to number.
				
				// Advance another character to check for ']'.
				char_count++;
				c = orig[char_count];
				if (c == ']') {
					end = char_count + 1;
					break;
				} else {
					printf("Variable '%s' index ending not found. Instead got "
						   "a '%c'.\n", var_name, c);
					exit(EXIT_FAILURE);
				}
			} else {
				// Variable name ended because of invalid name character.
				end = char_count;
			}
		}
	}
	
	// Argument ended with a variable name.
	if ((begin >= 0) && (end <= 0)) {
		end = char_count;
	}
	
	// Found a variable to substitute.
	if (begin >= 0) {
		// Get variable string representation for substitution.
		char strval[ARGUMENT_MAX_SIZE];
		variable_strval(var_name, index, strval);

#ifdef DEBUG
		printf("Substituting variable in string:\n%s\n", arg);
		printf("%*s^%*s^\t '%s' -> '%s'\n", begin, "", end - begin - 1, "",
			   var_name, strval);
#endif
		
		// Substitute the variable into the argument.
		arg[begin] = '\0';
		strcat(arg, strval);
		char_count = strlen(arg);
		for (uint8_t i = end; i < ARGUMENT_MAX_SIZE; i++) {
			arg[char_count++] = orig[i];
			
			if (orig[i] == '\0') {
				break;
			}
		}
		
#ifdef DEBUG
		printf("%s\n", arg);
#endif
		
		// Clean up our mess if we had to substitute a variable.
		sub_count++;
		free(orig);
	}
	
	// Substitute multiple variables if we had any matches this time.
	if (sub_count > 0) {
		sub_count += substitute_variables(command, arg);  // Yay, recursion!
	}

	return sub_count;
}

/**
 * Prints some debug information about a variable or layer.
 * Warning: This function alters the contents of "*thing".
 * 
 * @param  thing Thing to be inspected in string form.
 * @return       TRUE if the inspecting was successful.
 */
bool inspect(char *thing) {
	char type = thing[0];

	// Drop the thing's type symbol.
	for (size_t i = 1; ; i++) {
		thing[i - 1] = thing[i];
		
		if (thing[i] == '\0') {
			break;
		}
	}
	
	// Detect which kind of thing we are dealing with.
	if ((type == '$') || (type == '@') || (type == '&')) {
		// Variable
		variable_t *var = get_variable(thing);
		
		if (var == NULL) {
			printf("Variable '%c%s' not found.\n", type, thing);
			return false;
		} else {
			print_variable_info(*var);
		}
	} else if (type == 'l') {
		// Layer
		uint8_t layer_num = (uint8_t)atoi(thing);
		layer_t *layer = get_layer(layer_num);
		
		if (layer == NULL) {
			printf("Layer '%d' not found.\n", layer_num);
			return false;
		} else {
			print_layer_info(*layer);
		}
	} else {
		// Invalid
		printf("Invalid type of thing to inspect: '%c'.\n", type);
		return false;
	}
	
	return true;
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

	// Ignoring empty lines and comments.
	if ((line[0] == '\0') || (line[0] == '#')) {
		add_history_line(line);
		return true;
	}

	// Parse the line.
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
		} else if (strcmp("dimen", command) == 0) {
			// Dimension command.
			if (!create_dimension(argc, argv, false)) {
				return false;
			}
		} else if (strcmp("odimen", command) == 0) {
			// Offset dimension command.
			if (!create_dimension(argc, argv, true)) {
				return false;
			}
		} else if (strcmp("set", command) == 0) {
			// Command will set a variable.
			set_variable(argv[0], argv[1]);
#ifdef DEBUG
			print_variable_info(variables.list[variables.count - 1]);
#endif
		} else if (strcmp("layer", command) == 0) {
			// Set layer attributes command.
			set_layer((uint8_t)strtoul(argv[0], NULL, 10), argv[1], argv[2]);
		} else if (strcmp("list", command) == 0) {
			// List lines command.
			print_line_history();
		} else if (strcmp("inspect", command) == 0) {
			// Inspect command.
			if (!inspect(argv[0])) {
				return false;
			}
		} else {
			// Not a known command.
			printf("Unknown command '%s'.\n", command);
			return false;
		}
		
		// Cleaning up the arguments.
		for (uint8_t i = 0; i < argc; i++) {
			free(argv[i]);
		}

		// Add line to the history and return.
		add_history_line(line);
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
				exit(EXIT_FAILURE);
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
				exit(EXIT_FAILURE);
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
		exit(EXIT_FAILURE);
	}

#ifdef DEBUG
	printf("Number: %s - Unit: %s - Double: %f - Final: %ld\n", strnum, unit,
		   orig, num);
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
 * Checks if a command is of a type that shouldn't have its arguments variables
 * substituted by their values. This is mostly used for debugging commands.
 * 
 * @param  command Command name that should be checked.
 * @return         TRUE if variables in arguments shouldn't substituted.
 */
bool is_no_substitute_command(const char *command) {
	for (uint8_t i = 0; i < NOVARSUBS_COMMAND_SIZE; i++) {
		if (strcmp(command, nosubstitute_commands[i]) == 0) {
			return true;
		}
	}
	
	return false;
}

/**
 * Prints some debug information about a given object.
 *
 * @param object Object that you want to get information from.
 */
void print_object_info(const object_t object) {
	printf("Object Type: %d - %s\n", object.type,
		   valid_objects[object.type - 1]);
	printf("Coordinates (%d total):\n", object.coord_count);

	for (uint8_t i = 0; i < object.coord_count; i++) {
		printf("    %d. (%ld, %ld)\n", i, object.coord[i].x, object.coord[i].y);
	}
	
	print_layer_info(*get_layer(object.layer_num));
}

/**
 * Prints a list of all the lines in the history container.
 */
void print_line_history() {
	for (size_t i = 0; i < history.count; i++) {
		printf("%03lu: %s\n", i + 1, history.lines[i]);
	}
}

/**
 * Adds a line to the command history.
 * 
 * @param line Line to be added to the history.
 */
void add_history_line(const char *line) {
	// Dynamically add a new line to the array.
	history.lines = realloc(history.lines, sizeof(char*) * (history.count + 1));
	history.lines[history.count++] = strdup(line);
}

/**
 * Parses a command line and separates each part.
 *
 * @param  line      The command line without the newline character at the end.
 * @param  command   Pointer to a string that will contain the command after
 *                   parsing.
 * @param  arguments Array of strings that will contain the arguemnts.
 * @return           Number of arguments found for the command or -1 if there
 *                   was an error while parsing.
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
		
		// Ignore everything after the start of a comment.
		if (c == '#') {
			break;
		}

		// Treat each stage of parsing differently.
		switch (stage) {
		case PARSING_COMMAND:
			if ((c == ' ') || (c == '\t')) {
				// Space found, so the command part has ended.
				command[cur_cpos] = '\0';
				chomp(command);
				cur_cpos = 0;
				argc = 0;
				stage = PARSING_ARGUMENTS;
			} else {
				if ((cur_cpos + 1) < COMMAND_MAX_SIZE) {
					// Append characters to the command string.
					command[cur_cpos++] = c;
					command[cur_cpos] = '\0';
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
				chomp(cur_arg);
				substitute_variables(command, cur_arg);
				arguments[argc - 1] = strdup(cur_arg);
				cur_cpos = 0;
				cur_arg[0] = '\0';

				if (argc == ARGUMENT_ARRAY_MAX_SIZE) {
					printf("Maximum number of arguments exceeded.\n");
					return -1;
				}
			} else if ((c == ' ') || (c == '\t')) {
				// Ignoring the spaces or tabs.
			} else if (c == '=') {
				// We need to make sure we'll store this into a variable.
				chomp(cur_arg);
				substitute_variables(command, cur_arg);
				arguments[argc - 1] = strdup(cur_arg);
				cur_cpos = 0;
				cur_arg[0] = '\0';
				stage = PARSING_SET_OBJVAR;
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
		case PARSING_SET_OBJVAR:
			// Check if we are starting with the right variable type.
			if (cur_cpos == 0) {
				if ((c != ' ') && (c != '\t') && (c != '&')) {
					printf("Unknown first character for a object variable "
						   "'%c'\n", c);
					return -1;
				} else if (c == '&') {
					cur_arg[cur_cpos++] = c;
					cur_arg[cur_cpos] = '\0';
				}
			} else if ((c == ' ') || (c == '\t')) {
				// Reached a space. The variable must have ended.
				break;
			} else {
				// Get variable name.
				cur_arg[cur_cpos++] = c;
				cur_arg[cur_cpos] = '\0';
			}
			break;
		default:
			printf("ERROR: Unknown line parsing state. This shouldn't "
				   "happen.\n");
			return -1;
		}
	}

	// Look for unfinished stages.
	if (stage == PARSING_SET_OBJVAR) {
		// Leave the object variable as a argument to be dealt with later.
		argc++;
	} else if (stage == PARSING_COMMAND) {
		// Looks like we don't have any arguments.
		argc = 0;
	}

	// Store the last argument parsed.
	if (argc > 0) {
		chomp(cur_arg);

		if (stage == PARSING_ARGUMENTS) {
			substitute_variables(command, cur_arg);
		}

		arguments[argc - 1] = strdup(cur_arg);
	}

	return argc;
}

/**
 * Parses a nanoCAD formatted file.
 *
 * @param  filename Path to the file to be parsed.
 * @return          TRUE if everything went OK.
 */
bool parse_file(const char *filename) {
	// Open the CAD file for parsing.
	FILE *fp = fopen(filename, "r");
	if (fp == NULL) {
		printf("Couldn't open the CAD file: %s\n", filename);
		return false;
	}

	// Go through each line.
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	unsigned int linenum = 1;
	while ((read = getline(&line, &len, fp)) != -1) {
		// Remove the trailling newline.
		if (line[read - 1] == '\n') {
			line[read - 1] = '\0';
		}

#ifdef DEBUG
		if (linenum > 1) {
			printf("\n\n");
		}

		printf("Line %d: %s\n", linenum, line);
#endif

		// Parse lines.
		if (!parse_command(line)) {
			printf("Failed to parse line %d: %s\n", linenum, line);
			return false;
		}

		linenum++;
	}

	// Some clean-up.
	fclose(fp);
	free(line);

	return true;
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

/**
 * Frees up all of the memory used up in a array.
 * 
 * @param arr Array to have its items freed.
 * @param len Length of the array.
 */
void free_array(void **arr, const size_t len) {
	for (size_t i = 0; i < len; i++) {
		free(arr[i]);
	}
}

/**
 * Removes trailling whitespace from a string.
 * Warning: This function modifies the original string.
 * 
 * @param str String to be trimmed.
 */
void chomp(char *str) {
	for (int i = strlen(str) - 1; i >= 0; i--) {
		if (isspace(str[i])) {
			str[i] = '\0';
		}
	}
}

/**
 * Converts a hexadecimal number as string to a decimal 8-bit unsigned integer.
 * 
 * @param  hex Hexadecimal number as a string.
 * @return     8-bit unsigned integer.
 */
uint8_t hex_to_dec(const char *hex) {
	uint8_t dec = 0;
	uint8_t power = 0;
	
	for (int8_t digit = 1; digit >= 0; digit--) {
		if ((hex[digit] >= '0') && (hex[digit] <= '9')) {
			// Numbers.
			dec += (hex[digit] - '0') * (uint8_t)pow(16, power);
		} else if ((hex[digit] >= 'A') && (hex[digit] <= 'F')) {
			// Uppercase letters.
			dec += (hex[digit] - 'A' + 10) * (uint8_t)pow(16, power);
		} else if ((hex[digit] >= 'a') && (hex[digit] <= 'f')) {
			// Lowercase letters.
			dec += (hex[digit] - 'a' + 10) * (uint8_t)pow(16, power);
		} else {
			// Invalid character.
			printf("Invalid hexadecimal character '%c' in '%s'.\n",
				   hex[digit], hex);
			exit(EXIT_FAILURE);
		}
		
		power++;
	}
	
	return dec;
}

