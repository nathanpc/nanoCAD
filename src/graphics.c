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
bool graphics_init(const int width, const int height) {
	// Initialize SDL.
	int sdl_init_status = SDL_Init(SDL_INIT_EVERYTHING);

	if (sdl_init_status >= 0) {
		// Create a window.
		window = SDL_CreateWindow("nanoCAD", SDL_WINDOWPOS_CENTERED, 
				SDL_WINDOWPOS_CENTERED, width, height,
				SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);

		// Create the renderer.
		if (window != 0) {
			renderer = SDL_CreateRenderer(window, -1, 0);
		} else {
			printf("Couldn't create the SDL window.\n");
			return false;
		}
	} else {
		printf("There was an error while trying to initialize SDL: %d.\n",
				sdl_init_status);
		return false;
	}

	return true;
}

/**
 *  Clean the trash.
 */
void graphics_clean() {
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_Quit();
}
