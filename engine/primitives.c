/**
 * engine/primitives.c
 * Handles all of the operations related to the CAD engine's primitive objects.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#include "primitives.h"
#include <stdio.h>
#include "commons.h"

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
 * Sets the primitive object ID.
 *
 * @param obj Primitive object to be altered.
 * @param id  New ID of the object.
 *
 * @return ENGINE_OK if the operation was successful.
 */
engine_error_t primitive_set_id(primitive_obj_t *obj, long id) {
	obj->id = id;
	return ENGINE_OK;
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
 * Dumps the contents of a coordinate object to STDOUT as a JSON object.
 *
 * @param coord Coordinate object to be inspected.
 */
void coord_debug_print(coord_t coord) {
	printf(
		"{\n"
		"    \"x\": %ld,\n"
		"    \"y\": %ld\n"
		"}",
		coord.x, coord.y);
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
		"    \"id\": %ld,\n"
		"    \"type\": %u,\n"
		"    \"layer\": %u,\n"
		"    \"coords\": [\n",
		obj->id, obj->type, obj->layer);

	if (obj->coords) {
		coord_t *it;

		for (it = cvector_begin(obj->coords); it != cvector_end(obj->coords); ++it) {
			/* Dump the coordinate information. */
			printf("        [ %ld, %ld ]", it->x, it->y);

			/* Check if we still have layers to dump. */
			if ((it + 1) != cvector_end(obj->coords)) {
				printf(",\n");
			} else {
				printf("\n");
			}
		}
	}

	printf("    ]\n}");
}
