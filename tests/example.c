/**
 * test/example.c
 * A very simple, but complete, example of how to use this CAD engine.
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
	instance_t instance;
	layer_t *layer;

	/* Initialize some defaults. */
	err = ENGINE_OK;

	/* Initialize the engine instance. */
	engine_instance_init(&instance);

	/* Initialize a new layer. */
	layer = instance_layer_create(&instance);
	layer_set_name(layer, "Test Layer");
	layer_set_color_rgba(layer, 100, 150, 200, 255);

	/* Dump the engine instance information. */
	engine_instance_debug_print(&instance);
	printf("\n");

	/* Clean up. */
	engine_instance_init(&instance);

	return err;
}
