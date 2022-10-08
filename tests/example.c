/**
 * test/example.c
 * A very simple, but complete, example of how to use this CAD engine.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#include <stdio.h>
#include "../engine/engine.h"
#include "../graphics/sdl_graphics.h"

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
	primitive_obj_t *obj;

	/* Initialize some defaults. */
	err = ENGINE_OK;

	/* Initialize the engine instance. */
	engine_instance_init(&instance);

	/* Initialize a new layer. */
	layer = instance_layer_create(&instance);
	layer_set_name(layer, "Test Layer");
	layer_set_color_rgba(layer, 100, 150, 200, 255);

	/* Create a line. */
	obj = instance_object_create(&instance, PRIMITIVE_TYPE_LINE);
	primitive_add_coord(obj, coord_create(0, 0));
	primitive_add_coord(obj, coord_create(100, 0));
	primitive_add_coord(obj, coord_create(100, 100));
	primitive_add_coord(obj, coord_create(200, 100));

	/* Create a rectangle. */
	obj = instance_object_create(&instance, PRIMITIVE_TYPE_RECT);
	primitive_set_layer(obj, 1);
	primitive_add_coord(obj, coord_create(-100, -100));
	primitive_add_coord(obj, coord_create(0, 0));

	/* Create a circle. */
	obj = instance_object_create(&instance, PRIMITIVE_TYPE_CIRCLE);
	primitive_add_coord(obj, coord_create(-100, 100));
	primitive_add_coord(obj, coord_create(0, 0));

	/* Dump the engine instance information. */
	engine_instance_debug_print(&instance);
	printf("\n");

#ifndef MEMCHECK
	/* Initialize the graphics. */
	if (graphics_sdl_init(&instance, 600, 450)) {
		graphics_sdl_event_loop();
	} else {
		err = (engine_error_t)EXIT_FAILURE;
	}
#endif

	/* Clean up. */
	graphics_sdl_clean();
	engine_instance_free(&instance);

	return err;
}
