/**
 * test/example.c
 * A very simple example of how to use this CAD engine.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#include <stdio.h>
#include "../src/engine/engine.h"

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
	layer_t layer;

	/* Initialize some defaults. */
	err = ENGINE_OK;

	/* Initialize a new layer. */
	layer_new(&layer);
	layer_set_id(&layer, 1);
	layer_set_name(&layer, "Test Layer");
	layer_set_color_rgba(&layer, 100, 150, 200, 255);
	layer_debug_print(&layer);

	/* Clean up. */
	layer_free(&layer);

	return err;
}
