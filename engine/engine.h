/**
 * engine/engine.h
 * Handles the aggregation of all of the different parts of the engine into a
 * single instance.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#ifndef _ENGINE_ENGINE_H
#define _ENGINE_ENGINE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "commons.h"
#include "layers.h"

/* Engine instance structure definition. */
typedef struct {
	cvector_vector_type(layer_t) layers;
} instance_t;

/* Constructor and Destructor */
SHARED_API engine_error_t engine_instance_init(instance_t *instance);
SHARED_API engine_error_t engine_instance_free(instance_t *instance);

/* Layers */
SHARED_API layer_t *instance_layer_create(instance_t *instance);
SHARED_API layer_t *instance_layer_get_id(instance_t *instance, uint8_t id);

/* Debugging */
SHARED_API void engine_instance_debug_print(instance_t *instance);

#ifdef __cplusplus
}
#endif

#endif  /* _ENGINE_ENGINE_H */
