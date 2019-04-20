/**
 * nanocad.h
 * A tiny CAD engine with a super simple syntax designed to be easily embeddable.
 *
 * @author Nathan Campos <nathanpc@dreamintech.net>
 */

#ifndef _NANOCAD_H
#define _NANOCAD_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

// Constant definitions.
#define ENGINE_VERSION          "0.1a"
#define COMMAND_MAX_SIZE        15  // Yes, I'm lazy. Until further notice,
#define ARGUMENT_MAX_SIZE       30  // no dynamic-sized string and arrays for
#define ARGUMENT_ARRAY_MAX_SIZE 4   // you.

// Object type definitions.
#define TYPE_LINE   1
#define TYPE_RECT   2
#define TYPE_CIRCLE 3

// Coordinate structure.
typedef struct {
	long x;
	long y;
} coord_t;

// Object structure.
typedef struct {
	uint8_t type;
	uint8_t coord_count;
	coord_t *coord;
} object_t;

typedef struct {
	size_t count;
	object_t *list;
} object_container;

// General functions.
void nanocad_init();
bool parse_command(const char *line);
bool parse_file(const char *filename);

// Object manipulation.
object_t get_object(const size_t i);

// Debug functions.
void print_object_info(const object_t object);

#endif

