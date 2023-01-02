/**
 * cad_builtins.c
 * Some CAD built-in functions to integrate the Lisp interpreter and the CAD
 * engine.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#include "general_builtins.h"
#include <stdio.h>

#include "lisp.h"

/* Private Variables */
instance_t *cad_instance;

/* Private Methods */
coord_t coord_from_pair(atom_t pair);
bamboo_error_t builtin_cad_line(atom_t args, atom_t *result);

/**
 * Populates the environment with our built-in functions.
 *
 * @param env      Pointer to the environment to be populated.
 * @param instance CAD instance for us to interact with.
 *
 * @return BAMBOO_OK if the population was successful.
 */
bamboo_error_t builtins_setup_cad(env_t *env, instance_t *instance) {
	bamboo_error_t err;

	/* Store our CAD instance. */
	cad_instance = instance;

	/* Primitive objects creation. */
	IMPORT_BUILTIN("LINE", builtin_cad_line);

	return err;
}

/**
 * Quits the interpreter
 *
 * (line [p1] [p2] ...) -> object?
 *
 * @param p1  Starting point coordinate pair.
 * @param p2  Next/last point coordinate pair.
 * @param ... Next points coordinate pairs.
 */
bamboo_error_t builtin_cad_line(atom_t args, atom_t *result) {
	uint8_t numargs;
	primitive_obj_t *obj;

	/* Just in case... */
	numargs = 0;
	obj = NULL;
	*result = nil;

	/* Go through arguments. */
	while (!nilp(args)) {
		/* Check if we have a pair for the points. */
		if (car(args).type != ATOM_TYPE_PAIR) {
			*result = nil;
			return bamboo_error(BAMBOO_ERROR_WRONG_TYPE,
								"Point argument must be a pair.");
		}

		/* Create the line object if needed and add the next coordinate. */
		if (obj == NULL)
			obj = instance_object_create(cad_instance, PRIMITIVE_TYPE_LINE);
		primitive_add_coord(obj, coord_from_pair(car(args)));

		/* Ensure we are keeping track of the number of arguments passed. */
		numargs++;
		args = cdr(args);
	}

	/* Dump the engine instance information. */
	engine_instance_debug_print(cad_instance);
	printf("\n");

	/* Check if we have enough arguments to build a line. */
	if (numargs < 2)
		return BAMBOO_ERROR_ARGUMENTS;

	return BAMBOO_OK;
}

coord_t coord_from_pair(atom_t pair) {
	coord_t coord;

	coord.x = car(pair).value.integer;
	coord.y = car(cdr(pair)).value.integer;

	return coord;
}
