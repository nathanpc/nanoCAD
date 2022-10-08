/**
 * test/primitives.c
 * A very simple example of how to use primitives in the CAD engine.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#include <stdio.h>

#include "../engine/engine.h"

/**
 * Program's main entry point.
 *
 * @param argc Number of command-line arguments passed to the program.
 * @param argv Command-line arguments passed to the program.
 *
 * @return Exit code.
 */
int main(int argc, char **argv) {
	engine_error_t err;
	primitive_obj_t line;

	/* Initialize some defaults. */
	err = ENGINE_OK;

	/* Initialize a new line object. */
	primitive_new(&line);
	primitive_set_id(&line, 123);
	primitive_set_type(&line, PRIMITIVE_TYPE_LINE);
	primitive_add_coord(&line, coord_create(100, 200));
	primitive_add_coord(&line, coord_create(200, 300));
	primitive_debug_print(&line);
	printf("\n");

	/* Clean up. */
	primitive_free(&line);

	return err;
}
