/**
 * graphics/sdl_graphics.c
 * A SDL graphics abstraction layer.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#include "sdl_graphics.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "osifont.h"

/* Constants */
#define ZOOM_INTENSITY 10
#define FONT_SIZE 20
#define DIMENSION_TEXT_MAX_SIZE 20

/* Private Variables */
instance_t *cad_instance;
SDL_Window *window;
SDL_Renderer *renderer;
TTF_Font *font;
const uint8_t *keystates;
coord_t origin;
int zoom_level;
layer_t *current_layer;

/* Private Methods */
int is_key_down(SDL_Scancode key);
void set_origin(int x, int y);
void reset_origin();
int zoom(int percentage);
int change_layer(uint8_t id);
int draw_text(const char *text, coord_t pos, double angle);
int draw_line(coord_t start, coord_t end);
int draw_rect(coord_t start, coord_t end);
int draw_dimension(coord_t start, coord_t end, coord_t line_start,
				   coord_t line_end);
void graphics_render();

/**
 * Initializes the SDL graphics context.
 *
 * @param instance CAD engine instance.
 * @param width    Window width.
 * @param height   Window height.
 *
 * @return TRUE if the initialization went according to plan.
 */
int graphics_sdl_init(instance_t *instance, const int width, const int height) {
	/* Initialize some variables. */
	cad_instance = instance;
	current_layer = NULL;
	window = NULL;
	renderer = NULL;
	font = NULL;
	zoom_level = 100;

	/* Initialize SDL. */
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
		printf("There was an error while trying to initialize SDL: %s\n",
			   SDL_GetError());
		return 0;
	}

	/* Initialize SDL TTF module. */
	if (TTF_Init() < 0) {
		printf("There was an error while trying to initialize SDL_ttf: %s\n",
			   TTF_GetError());
		return 0;
	}

	/* Create a window. */
	window = SDL_CreateWindow("nanoCAD", SDL_WINDOWPOS_CENTERED, 
							  SDL_WINDOWPOS_CENTERED, width, height,
							  SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);

	/* Create the renderer. */
	if (window != 0) {
		renderer = SDL_CreateRenderer(window, -1, 0);
	} else {
		printf("Couldn't create the SDL window.\n");
		return 0;
	}
		
	/* Create the font object. */
	font = TTF_OpenFontRW(SDL_RWFromConstMem(osifont_ttf, osifont_ttf_length),
						  1, FONT_SIZE);
	if (font == NULL) {
		printf("Failed to load the embedded font. SDL_ttf Error: %s\n",
			   TTF_GetError());
		return 0;
	}

	/* Initialize the last bits. */
	change_layer(0);
	reset_origin();

	return 1;
}

/**
 *  Clean up the trash we've created.
 */
void graphics_sdl_clean() {
	/* Free our font. */
	TTF_CloseFont(font);
	font = NULL;

	/* Destroy window. */
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	window = NULL;
	renderer = NULL;
	
	/* Quit SDL subsystems. */
	TTF_Quit();
	SDL_Quit();
}

/**
 * Render the CAD graphics on screen.
 */
void graphics_render() {
	int ret = 0;

	/* Loop through each object and render it. */
	if (cad_instance->objects) {
		primitive_obj_t *it;
		ret = 0;

		for (it = cvector_begin(cad_instance->objects); it != cvector_end(cad_instance->objects); ++it) {
			uint8_t i = 0;

			/* Switch layers if needed by the object. */
			change_layer(it->layer);

			/* Do something different according to each type of object. */
			switch (it->type) {
				case PRIMITIVE_TYPE_LINE:
					/* Go through the coordinates drawing the lines. */
					if (it->coords) {
						for (i = 0; i < (cvector_size(it->coords) - 1); i++) {
							ret = draw_line(it->coords[i], it->coords[i + 1]);
						}
					}
					break;
				case PRIMITIVE_TYPE_RECT:
					/* Draw the rectangle. */
					ret = draw_rect(it->coords[0], it->coords[1]);
					break;
				case PRIMITIVE_TYPE_CIRCLE:
					break;
				default:
					printf("Invalid object type.\n");
					exit(EXIT_FAILURE);
			}

			/* Report any errors if there were any. */
			if (ret < 0) {
				printf("Error rendering line: %s\n", SDL_GetError());
			}
		}
	}
}

/**
 * Draws a line between two points.
 *
 * @param start Starting point for a line.
 * @param end   Ending point.
 *
 * @return SDL_RenderDrawLine return value.
 */
int draw_line(coord_t start, coord_t end) {
	int x1, y1, x2, y2;

	/* Transpose the coordinates to our own origin. */
	x1 = origin.x + start.x;
	y1 = origin.y - start.y;
	x2 = origin.x + end.x;
	y2 = origin.y - end.y;

	/* Draw the line. */
	return SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
}

/**
 * Draws a line between two points.
 *
 * @param start Starting point for the rectangle.
 * @param end   Opposite corner of the rectangle.
 *
 * @return SDL_RenderDrawLine return value.
 */
int draw_rect(coord_t start, coord_t end) {
	draw_line(start, coord_create(end.x, start.y));
	draw_line(coord_create(end.x, start.y), end);
	draw_line(end, coord_create(start.x, end.y));
	return draw_line(coord_create(start.x, end.y), start);
}

/**
 * Draws some text on the screen.
 *
 * @param text       The text to be rendered on screen.
 * @param pos        Where to put the text (Center-Center anchor).
 * @param angle      Which angle should the text be in.
 * @param compensate Should we compensate the rotation on width or height?
 *
 * @return SDL_RenderCopyEx return value.
 */
int draw_text(const char *text, coord_t pos, double angle) {
	int ret = 0;

	/* Transpose the coordinates to our own origin. */
	int x1 = origin.x + pos.x;
	int y1 = origin.y - pos.y;

	/* Create the text's surface. */
	SDL_Color color = { current_layer->color.r,
						current_layer->color.g,
						current_layer->color.b,
						current_layer->color.alpha };
	SDL_Surface *surface = TTF_RenderText_Solid(font, text, color);

	/* Create a texture for the text. */
	SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

	/* Get the text dimensions and free the surface. */
	int width = surface->w;
	int height = surface->h;
    SDL_FreeSurface(surface);

	/* Create text area rectangle. */
	SDL_Rect rect;
	rect.x = x1 - (width / 2);
	rect.y = y1 - (height / 2);
	rect.w = width;
	rect.h = height;

	/* Copy the texture to the renderer. */
	ret = SDL_RenderCopyEx(renderer, texture, NULL, &rect, angle, NULL,
						   SDL_FLIP_NONE);
	if (ret < 0)
		return ret;

	/* Destroy the temporary texture. */
	SDL_DestroyTexture(texture);

	return ret;
}

/**
 * Draws a dimension between two points.
 *
 * @param start      Starting point for the measurement.
 * @param end        Ending point for the measurement.
 * @param line_start Starting point for the dimension line.
 * @param line_end   Ending point for the dimension line.
 *
 * @return SDL_RenderDrawLine return value.
 */
int draw_dimension(const coord_t start, const coord_t end,
				   const coord_t line_start, const coord_t line_end) {
	int ret = 0;
	int pin_offset = 10;

	/* Transpose the coordinates to our own origin. */
	int x1 = origin.x + line_start.x;
	int y1 = origin.y - line_start.y;
	int x2 = origin.x + line_end.x;
	int y2 = origin.y - line_end.y;
	int sx = origin.x + start.x;
	int sy = origin.y - start.y;
	int ex = origin.x + end.x;
	int ey = origin.y - end.y;
	
	/* Make sure all dimension lines are going from left to right. */
	if (x1 > x2) {
		int xt = x1;
		int yt = y1;
		x1 = x2;
		y1 = y2;
		x2 = xt;
		y2 = yt;
	}
	
	/* Make sure all measured lines are going from left to right. */
	if (sx > ex) {
		int xt = sx;
		int yt = sy;
		sx = ex;
		sy = ey;
		ex = xt;
		ey = yt;
	}
	
	/* Draw the main dimension line. */
	ret = SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
	if (ret < 0) {
		return ret;
	}
	
	/* Text position variables will be the mid-point between marker pins. */
	int tx1 = 0;
	int ty1 = 0;
	int tx2 = 0;
	int ty2 = 0;
	
	/* Calculate the perpendicular line parameters. */
	int dx = x1 - x2;
	int dy = y1 - y2;
	int dist = (int)round(sqrt((dx * dx) + (dy * dy)));
	dx = (int)nearbyint((double)dx / dist);
	dy = (int)nearbyint((double)dy / dist);
	
	/* Draw the first marker pin. */
	int x3 = x1 + (pin_offset * dy);
	int y3 = y1 - (pin_offset * dx);
	int x4 = x1 - (pin_offset * dy);
	int y4 = y1 + (pin_offset * dx);
	ret = SDL_RenderDrawLine(renderer, x3, y3, x4, y4);
	if (ret < 0)
		return ret;

	/* Check the line direction and determine which marker position to use. */
	if ((sy > y1) && (ey > y2)) {
		/* Dimension line above measured line. */
		tx1 = x4;
		ty1 = y4;
		ty1 -= (int)((float)FONT_SIZE * 0.2);
	} else if ((sy < y1) && (ey < y2)) {
		/* Dimension line below the measured line. */
		tx1 = x3;
		ty1 = y3;
	} else if ((sx > x1) && (ex > x2)) {
		/* Dimension line to the left of measured line. */
		tx1 = x4;
		ty1 = y4;
		tx1 -= (int)((float)FONT_SIZE * 0.2);
	} else if ((sx < x1) && (ex < x2)) {
		/* Dimension line to the right of measured line. */
		tx1 = x4;
		ty1 = y4;
	}
	
	/* Draw the second marker pin. */
	x3 = x2 + (pin_offset * dy);
	y3 = y2 - (pin_offset * dx);
	x4 = x2 - (pin_offset * dy);
	y4 = y2 + (pin_offset * dx);
	ret = SDL_RenderDrawLine(renderer, x3, y3, x4, y4);
	if (ret < 0)
		return ret;
	
	/* Check the line direction and determine which marker position to use. */
	if ((sy > y1) && (ey > y2)) {
		/* Dimension line above measured line. */
		tx2 = x4;
		ty2 = y4;
		ty2 -= (int)((float)FONT_SIZE * 0.2);
	} else if ((sy < y1) && (ey < y2)) {
		/* Dimension line below the measured line. */
		tx2 = x3;
		ty2 = y3;
	} else if ((sx > x1) && (ex > x2)) {
		/* Dimension line to the left of measured line. */
		tx2 = x4;
		ty2 = y4;
		tx2 -= (int)((float)FONT_SIZE * 0.2);
	} else if ((sx < x1) && (ex < x2)) {
		/* Dimension line to the right of measured line. */
		tx2 = x4;
		ty2 = y4;
	}

	/* Build the measurement string. */
	/* TODO: Put the distance function inside the engine and return a string and
	         can have a unit set. */
	double distance = sqrt(pow(end.x - start.x, 2) + pow(end.y - start.y, 2));
	char text[DIMENSION_TEXT_MAX_SIZE];
	snprintf(text, DIMENSION_TEXT_MAX_SIZE, "%.0f", distance);
	
	/* Calculate text position with the origin reset. */
	coord_t text_pos;
    text_pos.x = ((tx1 + tx2) / 2) - origin.x;
	text_pos.y = origin.y - ((ty1 + ty2) / 2);

	/* Calculate the dimension text rotation angle. */
	double perpangle = atan2(y1 - y2, x1 - x2);
	double text_angle = perpangle * (180.0 / M_PI);
	
	/* Fix the rotation on opposite sides (remember that Y is inverted in SDL). */
	if ((sy > y1) && (ey > y2)) {
		/* Dimension line above measured line. */
		text_angle += 180;
	} else if ((sy < y1) && (ey < y2)) {
		/* Dimension line below the measured line. */
		text_angle += 180;
	} else if ((sx > x1) && (ex > x2)) {
		/* Dimension line to the left of measured line. */
		text_angle += 180;
	}
	
	/* Render the dimension text. */
	draw_text(text, text_pos, text_angle);
	
	return ret;
}

/**
 *  Render loop.
 */
void graphics_sdl_event_loop() {
	SDL_Event event;
	int zoom_amount;

	/* Initialize some variables. */
	keystates = SDL_GetKeyboardState(0);
	zoom_amount = 0;

	/* TODO: Handle touch events. */

	while (SDL_WaitEvent(&event)) {
		/* Set the background color and clear the window. */
		SDL_SetRenderDrawColor(renderer, 33, 40, 48, 255);
		SDL_RenderClear(renderer);

		switch (event.type) {
		case SDL_KEYDOWN:
			/* Keyboard key pressed. */
			if (is_key_down(SDL_SCANCODE_ESCAPE)) {
				/* Escape */
				SDL_Quit();
				exit(EXIT_SUCCESS);
			}
			break;
		case SDL_MOUSEMOTION:
			/* Mouse movement. */
			if (event.motion.state & SDL_BUTTON(SDL_BUTTON_LEFT)) {
				/* Pan around the view. */
				set_origin(origin.x + event.motion.xrel,
						   origin.y + event.motion.yrel);
			}
			break;
		case SDL_MOUSEWHEEL:
			/* Mouse wheel turned. */
			zoom_amount = zoom_level + (event.wheel.y * ZOOM_INTENSITY);
			zoom(zoom_amount);
#ifdef DEBUG
			printf("Zoom level: %d%%\n", zoom_amount);
#endif
			break;
		case SDL_WINDOWEVENT:
			/* Window stuff. */
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

		/* Update the graphics on the screen. */
		graphics_render();

		/* Show the window. */
		SDL_RenderPresent(renderer);
	}

	/* Clean up the house after the party. */
	graphics_sdl_clean();
}

/**
 * Sets the current zoom level.
 *
 * @param percentage Percentage of zoom to be applied to the viewport.
 * @return SDL_RenderSetScale return value.
 */
int zoom(int percentage) {
	zoom_level = percentage;
	float scale = (float)zoom_level / 100;

	return SDL_RenderSetScale(renderer, scale, scale);
}

/**
 * Changes the current layer we are drawing to. If it's the same as the current
 * one nothing is done.
 *
 * @param id New layer ID.
 * @return SDL_SetRenderDrawColor return value.
 */
int change_layer(uint8_t id) {
	layer_t *layer;

	/* Check if we don't have to change the current layer. */
	if (current_layer && (current_layer->id == id)) {
		layer = current_layer;
		goto setcolor;
	}

	/* Get the new layer. */
	layer = instance_layer_get_id(cad_instance, id);
	if (layer == NULL)
		return -1;
	current_layer = layer;

setcolor:
	/* Set the render color. */
	return SDL_SetRenderDrawColor(renderer, layer->color.r, layer->color.g,
								  layer->color.b, layer->color.alpha);
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

	/* Get window size and set the Y coordinate only. */
	SDL_GetWindowSize(window, NULL, &y);
	set_origin(0, y);
}

/**
 *  Check if a key is down.
 *
 *  @param key SDL_Scancode for the key you want to be tested.
 *  @return TRUE if the key is down.
 */
int is_key_down(SDL_Scancode key) {
	if (keystates != 0) {
		if (keystates[key] == 1) {
			return 1;
		} else {
			return 0;
		}
	}

	return 0;
}
