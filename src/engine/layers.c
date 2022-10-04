/**
 * engine/layers.c
 * Handles all of the operations related to the CAD engine's layers.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#include "layers.h"
#include <stdio.h>
#include <string.h>

/**
 * Initializes a brand new layer structure.
 * 
 * @param layer Layer structure to be initialized.
 * @return ENGINE_OK if the operation was successful.
 */
engine_error_t layer_new(layer_t *layer) {
	/* Initialize the layer number and name to "uninitialized" defaults. */
	layer->id = -1;
	layer->name = NULL;
	
	/* Initialize the color structure. */
	layer->color.r = 255;
	layer->color.g = 255;
	layer->color.b = 255;
	layer->color.alpha = 255;

	return ENGINE_OK;
}

/**
 * Frees up any resources allocated by the layer structure and invalidates it.
 * 
 * @param layer Layer to be free'd.
 * @return ENGINE_OK if the operation was successful.
 */
engine_error_t layer_free(layer_t *layer) {
	/* Invalidate the ID. */
	layer->id = -1;

	/* Free up the name. */
	if (layer->name)
		free(layer->name);
	layer->name = NULL;

	return ENGINE_OK;
}

/**
 * Sets the layer ID.
 * 
 * @param layer Layer to be altered.
 * @param id    New layer ID.
 *
 * @return ENGINE_OK if the operation was successful.
 */
engine_error_t layer_set_id(layer_t *layer, uint8_t id) {
	layer->id = (int16_t)id;
	return ENGINE_OK;
}

/**
 * Sets the layer name.
 * 
 * @param layer Layer to be altered.
 * @param name  New layer name.
 *
 * @return ENGINE_OK if the operation was successful. 
 */
engine_error_t layer_set_name(layer_t *layer, const char *name) {
	/* Allocate the memory to store the layer name. */
	layer->name = (char *)realloc(layer->name, strlen(name) * sizeof(char));
	if (layer->name == NULL)
		return ENGINE_ERROR_ALLOC_FAIL;
	
	/* Copy the name over. */
	strcpy(layer->name, name);

	return ENGINE_OK;
}

/**
 * Sets the layer color.
 * 
 * @param layer Layer to be altered.
 * @param color New layer color.
 *
 * @return ENGINE_OK if the operation was successful. 
 */
engine_error_t layer_set_color(layer_t *layer, rgba_color_t color) {
	layer->color = color;
	return ENGINE_OK;
}

/**
 * Sets the layer color using discrete RGBA values.
 * 
 * @param layer Layer to be altered.
 * @param red   Red color value.
 * @param green Green color value.
 * @param blue  Blue color value.
 * @param alpha Alpha value.
 *
 * @return ENGINE_OK if the operation was successful. 
 */
engine_error_t layer_set_color_rgba(layer_t *layer, uint8_t red, uint8_t green,
		uint8_t blue, uint8_t alpha) {
	layer->color.r = red;
	layer->color.g = green;
	layer->color.b = blue;
	layer->color.alpha = alpha;

	return ENGINE_OK;
}

/**
 * Dumps the contents of a layer to STDOUT as a JSON object.
 * 
 * @param layer Layer to be inspected.
 */
void layer_debug_print(const layer_t *layer) {
	/* I know this looks absolutely horrible. Stop staring at it! */
	printf(
		"{\n"
		"    \"id\": %d,\n"
		"    \"name\": \"%s\",\n"
		"    \"color\": {\n"
		"        \"r\": %u,\n"
		"        \"g\": %u,\n"
		"        \"b\": %u,\n"
		"        \"alpha\": %u\n"
		"    }\n"
		"}\n",
		layer->id, layer->name, layer->color.r, layer->color.g, layer->color.b,
		layer->color.alpha);
}
