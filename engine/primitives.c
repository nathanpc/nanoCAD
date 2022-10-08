/**
 * engine/primitives.c
 * Handles all of the operations related to the CAD engine's primitive objects.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#include "primitives.h"
#include <stdio.h>

/**
 * Initializes a brand new primitive object structure.
 * 
 * @param obj Primitive object structure to be initialized.
 * @return ENGINE_OK if the operation was successful.
 */
engine_error_t primitive_new(primitive_obj_t *obj) {
	obj->type = PRIMITIVE_TYPE_INVALID;
	obj->layer = 0;
	obj->coords = NULL;

	return ENGINE_OK;
}

/**
 * Frees up any resources allocated by the primitive object structure and
 * invalidates it.
 * 
 * @param obj Primitive object structure to be free'd.
 * @return ENGINE_OK if the operation was successful.
 */
engine_error_t primitive_free(primitive_obj_t *obj) {
	/* Invalidate the object. */
	obj->type = PRIMITIVE_TYPE_INVALID;
	obj->layer = 0;

	/* Free up the coordinates vector. */
	cvector_free(obj->coords);
	obj->coords = NULL;

	return ENGINE_OK;
}

/**
 * Create a coordinate structure from X and Y values.
 * 
 * @param x X position.
 * @param y Y position.
 *
 * @return Pre-populated coordinate structure.
 */
coord_t coord_create(long x, long y) {
	coord_t coord;

	coord.x = x;
	coord.y = y;

	return coord;
}

/**
 * Sets the primitive object type.
 * 
 * @param obj  Primitive object to be altered.
 * @param type New primitive object type.
 *
 * @return ENGINE_OK if the operation was successful.
 */
engine_error_t primitive_set_type(primitive_obj_t *obj, primitive_type_t type) {
	obj->type = type;
	return ENGINE_OK;
}

/**
 * Sets the primitive object layer.
 * 
 * @param obj   Primitive object to be altered.
 * @param layer New primitive object layer.
 *
 * @return ENGINE_OK if the operation was successful.
 */
engine_error_t primitive_set_layer(primitive_obj_t *obj, uint8_t layer) {
	obj->layer = layer;
	return ENGINE_OK;
}

/**
 * Appends a coordinate to the primitive object structure.
 * 
 * @param obj   Primitive object to be altered.
 * @param coord New coordinate to be appended to the primitive object.
 *
 * @return ENGINE_OK if the operation was successful.
 */
engine_error_t primitive_add_coord(primitive_obj_t *obj, coord_t coord) {
	cvector_push_back(obj->coords, coord);
	return ENGINE_OK;
}

/**
 * Dumps the contents of a primitive object to STDOUT as a JSON object.
 * 
 * @param obj Primitive object to be inspected.
 */
void primitive_debug_print(const primitive_obj_t *obj) {
	/* I know this looks absolutely horrible. Stop staring at it! */
	printf(
		"{\n"
		"    \"type\": %u,\n"
		"    \"layer\": %u,\n"
		"    \"coords\": [\n",
		obj->type, obj->layer);

/*		"        \"r\": %u,\n" */

	printf("    ]\n}");
}
