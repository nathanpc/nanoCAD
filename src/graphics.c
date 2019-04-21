/**
 * graphics.c
 * A SDL graphics abstraction layer.
 *
 * @author Nathan Campos <nathanpc@dreamintech.net>
 */

#include <SDL.h>
#include <stdlib.h>
#include <stdio.h>
#include "nanocad.h"
#include "graphics.h"

// SDL context.
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
const uint8_t *keystates;
bool running = false;

// Internal functions.
bool is_key_down(const SDL_Scancode key);
void graphics_render();
void graphics_eventloop();

/**
 * Initializes the SDL graphics context.
 *
 * @param  width  Window width.
 * @param  height Window height.
 * @return        TRUE if the initialization went according to plan.
 */
bool graphics_init(const int width, const int height) {
	// Initialize SDL.
	int sdl_init_status = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

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

	running = true;
	return true;
}

/**
 *  Clean the trash.
 */
void graphics_clean() {
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_Quit();
	running = false;
}

/**
 * Render the CAD graphics on screen.
 */
void graphics_render() {
	// Get the object container from the engine.
	object_container objects;
	get_container(&objects);

	// Loop through each object and render it.
	for (size_t i = 0; i < objects.count; i++) {
		object_t obj = objects.list[i];
		SDL_SetRenderDrawColor(renderer, 69, 69, 69, 255);  // Set render color.

		switch (obj.type) {
			case TYPE_LINE:
				if (SDL_RenderDrawLine(renderer, obj.coord[0].x, obj.coord[0].y,
							obj.coord[1].x, obj.coord[1].y) < 0) {
					printf("Error rendering line: %s\n", SDL_GetError());
				}
				break;
			default:
				printf("Invalid object type.\n");
				exit(EXIT_FAILURE);
		}
	}
}

/**
 *  Render loop.
 */
void graphics_eventloop() {
	keystates = SDL_GetKeyboardState(0);
	SDL_Event event;

	while (running && SDL_WaitEvent(&event)) {
		// Set the background color and clear the window.
		SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255);
		SDL_RenderClear(renderer);

		switch (event.type) {
			case SDL_KEYDOWN:
				if (is_key_down(SDL_SCANCODE_ESCAPE)) {
					// Escape
					SDL_Quit();
					exit(EXIT_SUCCESS);
				}
				break;
			case SDL_MOUSEMOTION:
				break;
			case SDL_WINDOWEVENT:
				switch (event.window.event) {
					case SDL_WINDOWEVENT_RESIZED:
#ifdef DEBUG
						SDL_Log("Window %d resized to %dx%d",
								event.window.windowID, event.window.data1,
								event.window.data2);
#endif
						break;
				}
				break;
		}

		// Update the graphics on the screen.
		graphics_render();

		// Show the window.
		SDL_RenderPresent(renderer);
	}

	// Clean up the house after the party.
	graphics_clean();
}

/**
 *  Check if a key is down.
 *
 *  @param  key SDL_Scancode for the key you want to be tested.
 *  @return     TRUE if the key is down.
 */
bool is_key_down(const SDL_Scancode key) {
	if (keystates != 0) {
		if (keystates[key] == 1) {
			return true;
		} else {
			return false;
		}
	}

	return false;
}
