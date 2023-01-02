/**
 * engine/engine.c
 * Handles the aggregation of all of the different parts of the engine into a
 * single instance.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#include "engine.h"
#include <stdio.h>
#include "primitives.h"

/* Private Methods */
void cvector_free_layer(layer_t layer);
void cvector_free_primitive(primitive_obj_t obj);

/**
 * Initializes a brand new engine instance.
 * 
 * @param instance Engine instance structure to be initialized.
 * @return ENGINE_OK if the operation was successful.
 */
engine_error_t engine_instance_init(instance_t *instance) {
	layer_t def_layer;

	/* Ensure some default values are applied. */
	instance->layers = NULL;
	instance->objects = NULL;

	/* Create the default 0 layer. */
	layer_new(&def_layer);
	layer_set_id(&def_layer, 0);
	layer_set_color_rgba(&def_layer, 255, 255, 255, 255);
	layer_set_name(&def_layer, "Default");
	cvector_push_back(instance->layers, def_layer);

	return ENGINE_OK;
}

/**
 * Frees up any resources allocated by the engine instance.
 * 
 * @param instance Engine instance structure to be free'd.
 * @return ENGINE_OK if the operation was successful.
 */
engine_error_t engine_instance_free(instance_t *instance) {
	/* Free up all of the cvector stuff. */
	cvector_free_each_and_free(instance->layers, cvector_free_layer);
	instance->layers = NULL;
	cvector_free_each_and_free(instance->objects, cvector_free_primitive);
	instance->objects = NULL;

	return ENGINE_OK;
}

/**
 * Creates a brand new layer in the engine instance and returns it.
 * 
 * @param instance Engine instance.
 * @return Brand new layer we've just created.
 */
layer_t *instance_layer_create(instance_t *instance) {
	layer_t layer;

	/* Create a brand new layer with some defaults. */
	layer_new(&layer);
	layer_set_id(&layer, (cvector_last(instance->layers)).id + 1);
	layer_set_color_rgba(&layer, 255, 255, 255, 255);
	cvector_push_back(instance->layers, layer);

	return &(cvector_last(instance->layers));
}

/**
 * Gets a layer from an engine instance by its ID.
 *
 * @param instance Engine instance.
 * @param id       ID of the layer you want to get.
 *
 * @return Layer with the corresponding ID or NULL if it wasn't found.
 */
layer_t *instance_layer_get_id(instance_t *instance, uint8_t id) {
	layer_t *it;

	/* Go through the layers searching for one with a matching ID. */
	for (it = cvector_begin(instance->layers); it != cvector_end(instance->layers); ++it) {
		if (it->id == id)
			return it;
	}

	return NULL;
}

/**
 * Creates a brand new object in the engine instance and returns it.
 *
 * @param instance Engine instance.
 * @param type     New object type.
 *
 * @return Brand new object we've just created.
 */
primitive_obj_t *instance_object_create(instance_t *instance,
										primitive_type_t type) {
	primitive_obj_t object;
	uint8_t id = 1;
	if (instance->objects)
		id = (cvector_last(instance->objects)).id + 1;

	/* Create a brand new object with some defaults. */
	primitive_new(&object);
	primitive_set_id(&object, id);
	primitive_set_type(&object, type);
	primitive_set_layer(&object, 0);
	cvector_push_back(instance->objects, object);

	return &(cvector_last(instance->objects));
}

/**
 * Dumps the contents of an engine instance to STDOUT as a JSON object.
 *
 * @param instance Engine instance structure to be inspected.
 */
void engine_instance_debug_print(instance_t *instance) {
#ifdef DEBUG
	printf("{\n");

	/* Go through the layers. */
	printf("    \"layers\": [\n");
	if (instance->layers) {
		layer_t *it;

		for (it = cvector_begin(instance->layers); it != cvector_end(instance->layers); ++it) {
			/* Dump the layer information. */
			layer_debug_print(it);

			/* Check if we still have layers to dump. */
			if ((it + 1) != cvector_end(instance->layers)) {
				printf(",\n");
			} else {
				printf("\n");
			}
		}
	}
	printf(
		"    ],\n"
		"    \"objects\": [\n");

	/* Go through the objects. */
	if (instance->objects) {
		primitive_obj_t *it;

		for (it = cvector_begin(instance->objects); it != cvector_end(instance->objects); ++it) {
			/* Dump the object information. */
			primitive_debug_print(it);

			/* Check if we still have objects to dump. */
			if ((it + 1) != cvector_end(instance->objects)) {
				printf(",\n");
			} else {
				printf("\n");
			}
		}
	}
	printf("    ]\n}");
#endif
}

/**
 * CVector layer destructor function.
 * 
 * @param layer Layer structure to have its contents free'd.
 */
void cvector_free_layer(layer_t layer) {
	layer_free(&layer);
}

/**
 * CVector primitive object destructor function.
 *
 * @param obj Primitive object structure to have its contents free'd.
 */
void cvector_free_primitive(primitive_obj_t obj) {
	primitive_free(&obj);
}
