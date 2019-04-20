/**
 * graphics.c
 * A SDL graphics abstraction layer.
 *
 * @author Nathan Campos <nathanpc@dreamintech.net>
 */

#include <SDL.h>
#include "graphics.h"

// SDL context.
SDL_Window *window;
SDL_Renderer *renderer;

/**
 * Initializes the SDL graphics context.
 */
void graphics_init() {
	// Initialize SDL.
	int sdl_init_status = SDL_Init(SDL_INIT_EVERYTHING);

	if (sdl_init_status >= 0) {
		// Create a window.
		window = SDL_CreateWindow("nanoCAD", SDL_WINDOWPOS_CENTERED, 
				SDL_WINDOWPOS_CENTERED, 600, 450,
				SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);

		// Create the renderer.
		if (window != 0) {
			renderer = SDL_CreateRenderer(window, -1, 0);
		} else {
			printf("Couldn't create the SDL window.\n");
			exit(1);
		}
	} else {
		printf("There was an error while trying to initialize SDL: %d.\n",
				sdl_init_status);
		exit(1);
	}
}

