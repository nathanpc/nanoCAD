/**
 * engine/primitives.h
 * Handles all of the operations related to the CAD engine's primitive objects.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#ifndef _ENGINE_PRIMITIVES_H
#define _ENGINE_PRIMITIVES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>

#include "commons.h"

/* Primitive object type definitions. */
typedef enum {
	PRIMITIVE_TYPE_INVALID = 0,
	PRIMITIVE_TYPE_LINE,
	PRIMITIVE_TYPE_RECT,
	PRIMITIVE_TYPE_CIRCLE
} primitive_type_t;

/* Coordinate structure. */
typedef struct {
	long x;
	long y;
} coord_t;

/* Primitive object structure. */
typedef struct {
	long id;
	primitive_type_t type;
	uint8_t layer;
	cvector_vector_type(coord_t) coords;
} primitive_obj_t;

/* Constructor and Destructor */
SHARED_API engine_error_t primitive_new(primitive_obj_t *obj);
SHARED_API engine_error_t primitive_free(primitive_obj_t *obj);

/* Convenience Functions for Coordinates */
SHARED_API coord_t coord_create(long x, long y);

/* Setters */
SHARED_API engine_error_t primitive_set_id(primitive_obj_t *obj,
										   long id);
SHARED_API engine_error_t primitive_set_type(primitive_obj_t *obj,
											 primitive_type_t type);
SHARED_API engine_error_t primitive_set_layer(primitive_obj_t *obj,
											  uint8_t layer);
SHARED_API engine_error_t primitive_add_coord(primitive_obj_t *obj,
											  coord_t coord);

/* Debugging */
SHARED_API void coord_debug_print(coord_t coord);
SHARED_API void primitive_debug_print(const primitive_obj_t *obj);

#ifdef __cplusplus
}
#endif

#endif  /* _ENGINE_PRIMITIVES_H */
