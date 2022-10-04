/**
 * engine/layers.h
 * Handles all of the operations related to the CAD engine's layers.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#ifndef _ENGINE_LAYERS_H
#define _ENGINE_LAYERS_H

#include <stdint.h>
#include <stdlib.h>
#include <cvector.h>

#include "commons.h"

/* RGBA color structure. */
typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t alpha;
} rgba_color_t;

/* Layer structure. */
typedef struct {
	int16_t id;
	char *name;
	rgba_color_t color;
} layer_t;

/* Constructor and Destructor */
engine_error_t layer_new(layer_t *layer);
engine_error_t layer_free(layer_t *layer);

/* Setters */
engine_error_t layer_set_id(layer_t *layer, uint8_t id);
engine_error_t layer_set_name(layer_t *layer, const char *name);
engine_error_t layer_set_color(layer_t *layer, rgba_color_t color);
engine_error_t layer_set_color_rgba(layer_t *layer, uint8_t red, uint8_t green,
	uint8_t blue, uint8_t alpha);

/* Debugging */
void layer_debug_print(const layer_t *layer);

#endif  /* _ENGINE_LAYERS_H */
