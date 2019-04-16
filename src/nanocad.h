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

// Constant definitions.
#define ENGINE_VERSION   "0.1a"
#define COMMAND_MAX_SIZE        15  // Yes, I'm lazy. Until further notice,
#define ARGUMENT_MAX_SIZE       30  // no dynamic-sized string and arrays for
#define ARGUMENT_ARRAY_MAX_SIZE 4   // you.

// Function prototypes.
int parse_line(const char *line, char *command, char **arguments);

#endif

