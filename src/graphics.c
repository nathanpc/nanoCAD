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

// Constants
#define ZOOM_INTENSITY 10

// SDL context.
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
const uint8_t *keystates;
bool running = false;
coord_t origin;
int zoom_level = 100;

// Internal functions.
bool is_key_down(const SDL_Scancode key);
void set_origin(const int x, const int y);
void reset_origin();
void zoom(const int percentage);
int draw_line(const coord_t start, const coord_t end);
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
	running = false;

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

	// Initialize variables.
	running = true;
	reset_origin();

	return true;
}

/**
 *  Clean the trash.
 */
void graphics_clean() {
	running = false;

	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_Quit();
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
		int ret = 0;

		// Do something different according to each type of object.
		switch (obj.type) {
		case TYPE_LINE:
			ret = draw_line(obj.coord[0], obj.coord[1]);
			break;
		default:
			printf("Invalid object type.\n");
			exit(EXIT_FAILURE);
		}

		// Report any errors if there were any.
		if (ret < 0) {
			printf("Error rendering line: %s\n", SDL_GetError());
		}
	}
}

/**
 * Draws a line between two points.
 *
 * @param  start Starting point for the line.
 * @param  end   Ending point.
 * @return       SDL_RenderDrawLine return value.
 */
int draw_line(const coord_t start, const coord_t end) {
	// Transpose the coordinates to our own origin.
	int x1 = origin.x + start.x;
	int y1 = origin.y - start.y;
	int x2 = origin.x + end.x;
	int y2 = origin.y - end.y;

	// TODO: Change color by layer.
	SDL_SetRenderDrawColor(renderer, 69, 69, 69, 255);
	return SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
}

/**
 *  Render loop.
 */
void graphics_eventloop() {
	keystates = SDL_GetKeyboardState(0);
	SDL_Event event;
	int zoom_amount = 0;

	// TODO: Handle touch events.

	while (running && SDL_WaitEvent(&event)) {
		// Set the background color and clear the window.
		SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255);
		SDL_RenderClear(renderer);

		switch (event.type) {
		case SDL_KEYDOWN:
			// Keyboard key pressed.
			if (is_key_down(SDL_SCANCODE_ESCAPE)) {
				// Escape
				SDL_Quit();
				exit(EXIT_SUCCESS);
			}
			break;
		case SDL_MOUSEMOTION:
			// Mouse movement.
			if (event.motion.state & SDL_BUTTON(SDL_BUTTON_LEFT)) {
				// Pan around the view.
				set_origin(origin.x + event.motion.xrel,
						   origin.y + event.motion.yrel);
			}
			break;
		case SDL_MOUSEWHEEL:
			// Mouse wheel turned.
			zoom_amount = zoom_level + (event.wheel.y * ZOOM_INTENSITY);
			zoom(zoom_amount);
#ifdef DEBUG
			printf("Zoom level: %d%%\n", zoom_amount);
#endif
			break;
		case SDL_WINDOWEVENT:
			// Window stuff.
			switch (event.window.event) {
			case SDL_WINDOWEVENT_RESIZED:
#ifdef DEBUG
				SDL_Log("Window %d resized to %dx%d",
						event.window.windowID, event.window.data1,
						event.window.data2);
#endif

				reset_origin();
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
 * Sets the current zoom level.
 *
 * @param percentage Percentage of zoom to be applied to the viewport.
 */
void zoom(const int percentage) {
	zoom_level = percentage;
	float scale = (float)zoom_level / 100;
	SDL_RenderSetScale(renderer, scale, scale);
}

/**
 * Sets a new origin point relative to the SDL origin.
 *
 * @param x New x coordinate.
 * @param y New y coordinate.
 */
void set_origin(const int x, const int y) {
	origin.x = x;
	origin.y = y;

#ifdef DEBUG
	printf("New origin set: (%d, %d)\n", (int)origin.x, (int)origin.y);
#endif
}

/**
 * Resets the origin back to a more cartesian place.
 */
void reset_origin() {
	int y = 0;

	// Get window size and set the Y coordinate only.
	SDL_GetWindowSize(window, NULL, &y);
	set_origin(0, y);
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
