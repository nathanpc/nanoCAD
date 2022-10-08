/**
 * graphics/sdl_graphics.h
 * A SDL graphics abstraction layer.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#ifndef _GRAPHICS_SDL_GRAPHICS_H
#define _GRAPHICS_SDL_GRAPHICS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../engine/engine.h"

/* Initialization and Destruction */
int graphics_sdl_init(instance_t *instance, int width, int height);
void graphics_sdl_clean();

/* Event Loop */
void graphics_sdl_event_loop();

#ifdef __cplusplus
}
#endif

#endif /* _GRAPHICS_SDL_GRAPHICS_H */
