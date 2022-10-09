/**
 * general_builtins.c
 * Some general built-in functions to get some more functionality to the Lisp
 * interpreter.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#include "general_builtins.h"
#include <stdio.h>

#include "lisp.h"

/* Private Methods */
bamboo_error_t builtin_quit(atom_t args, atom_t *result);
bamboo_error_t builtin_load(atom_t args, atom_t *result);

/**
 * Populates the environment with our built-in functions.
 *
 * @param env Pointer to the environment to be populated.
 * @return BAMBOO_OK if the population was successful.
 */
bamboo_error_t builtins_populate_general(env_t *env) {
	bamboo_error_t err;

	/* Quit interpreter. */
	IMPORT_BUILTIN("QUIT", builtin_quit);
	IMPORT_BUILTIN("EXIT", builtin_quit);

	/* Load source file. */
	IMPORT_BUILTIN("LOAD", builtin_load);

	return err;
}

/**
 * Quits the interpreter
 *
 * (quit [retval])
 *
 * @param retval Value to be returned when exiting the interpreter.
 */
bamboo_error_t builtin_quit(atom_t args, atom_t *result) {
	atom_t retval;

	/* Just in case... */
	*result = bamboo_int(-1);

	/* Check if we don't have any arguments. */
	if (nilp(args)) {
		result->value.integer = 0;
		printf("Bye!" LINEBREAK);

		return (bamboo_error_t)BAMBOO_REPL_QUIT;
	}

	// Check if we have more than a single argument.
	if (!nilp(cdr(args)))
		return BAMBOO_ERROR_ARGUMENTS;

	// Get the first argument.
	retval = car(args);

	// Check if its the right type of argument.
	if (retval.type != ATOM_TYPE_INTEGER)
		return BAMBOO_ERROR_WRONG_TYPE;

	// Exit with the specified return value.
	*result = retval;
	_tprintf("Bye!" LINEBREAK);
	return (bamboo_error_t)BAMBOO_REPL_QUIT;
}

/**
 * Evaluates the contents of a file.
 *
 * (load fname) -> any?
 *
 * @param  fname Path to the file to be loaded into the environment.
 * @return       Last return value of the evaluated source.
 */
bamboo_error_t builtin_load(atom_t args, atom_t *result) {
	atom_t fname;

	// Just in case...
	*result = nil;

	// Check if we don't have any arguments.
	if (nilp(args)) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
							"A file path must be supplied to this function");
	}

	// Check if we have more than a single argument.
	if (!nilp(cdr(args))) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
							"Only a single file path should be supplied to this function");
	}

	// Get the file name argument.
	fname = car(args);

	// Check if its the right type of argument.
	if (fname.type != ATOM_TYPE_STRING) {
		return bamboo_error(BAMBOO_ERROR_WRONG_TYPE,
							"File name atom must be of type string");
	}

	// Load the file.
	return lisp_load_source(bamboo_get_root_env(), *fname.value.str, result);
}
