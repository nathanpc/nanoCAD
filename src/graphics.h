/**
 * graphics.h
 * A SDL graphics abstraction layer.
 *
 * @author Nathan Campos <nathanpc@dreamintech.net>
 */

#ifndef _GRAPHICS_H
#define _GRAPHICS_H

#include <stdbool.h>

// Initialization and destruction.
bool graphics_init(const int width, const int height);
void graphics_clean();

// Event loop.
void graphics_eventloop();

#endif

