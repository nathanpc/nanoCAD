/**
 * main.c
 * Fully-featured REPL and interpreter for Bamboo Lisp integrated with the CAD
 * engine.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#include <locale.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/_pthread/_pthread_t.h>
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#include "../lisp/repl/common/tgetopt.h"
#endif /* _WIN32 */
#if defined(__linux__) || defined(__APPLE__) || defined(__unix__)
#include <unistd.h>
#include <getopt.h>
#endif /* __linux__ || __APPLE__ || __unix__ */
#include <pthread.h>

#include "../engine/engine.h"
#include "../graphics/sdl_graphics.h"
#include "../lisp/lisp.h"
#include "input.h"

/* Private Definitions */
#define REPL_INPUT_MAX_LEN 512

/* Private Variables */
static instance_t cad_instance;
static env_t repl_env;
static char repl_input[REPL_INPUT_MAX_LEN + 1];
static bool env_initialized;
static pthread_mutex_t engine_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Private Methods */
void *graphics_subsystem(void *ptr);
void enable_unicode(void);
void usage(const char *pname, int retval);
void parse_args(int argc, char **argv);
bamboo_error_t init_env(void);
bamboo_error_t destroy_env(void);
void *repl(void *ptr);
void load_include(const char *fname, bool terminate);
void run_source(const char *fname);
void cleanup(void);

/**
 * Program's main entry point.
 *
 * @param argc Number of command-line arguments passed to the program.
 * @param argv Command-line arguments passed to the program.
 *
 * @return 0 if everything went fine.
 */
int main(int argc, char *argv[]) {
	bamboo_error_t bamboo_err;
	engine_error_t engine_err;
	pthread_t repl_thread;
	int ret_trepl;

	/* Setup some flags. */
	bamboo_err = BAMBOO_OK;
	engine_err = ENGINE_OK;
	env_initialized = false;
	ret_trepl = 0;

	/* Make sure we clean things up. */
	atexit(cleanup);

	/* Enable Unicode support in the console and parse any given arguments. */
	enable_unicode();
	parse_args(argc, argv);

	/* Initialize the Lisp environment. */
	if (!env_initialized) {
		bamboo_err = init_env();
		IF_BAMBOO_ERROR(bamboo_err)
			goto quit;
	}

	/* Initialize the CAD engine instance. */
	engine_err = engine_instance_init(&cad_instance);
	if (engine_err != ENGINE_OK) {
		fprintf(stderr, "CAD engine failed to initialize with code %d\n", engine_err);
		goto quit;
	}

	/* Create the thread for the REPL. */
	ret_trepl = pthread_create(&repl_thread, NULL, repl, NULL);
	if (ret_trepl) {
		fprintf(stderr, "Failed to create the REPL thread. (%d)\n", ret_trepl);
		goto quit;
	}

	/* Start the graphics subsystem. */
	graphics_subsystem(NULL);

	/* Join the threads and clean things up. */
	pthread_join(repl_thread, NULL);
quit:
	cleanup();
	return bamboo_err;
}

/**
 * Function that will deal with the graphics subsystem in a different thread.
 *
 * @param ptr Parameter passed to the thread upon creation.
 * @return Thread return value.
 */
void *graphics_subsystem(void *ptr) {
	int err = 0;

	/* Initialize the graphics. */
	if ((err = graphics_sdl_init(&cad_instance, 600, 450))) {
		/* Run the graphics event loop. */
		graphics_sdl_event_loop();
	} else {
		fprintf(stderr, "Graphics subsystem failed to initialize with code: %d\n", err);
	}

	return NULL;
}

/**
 * Initializes the Lisp environment.
 *
 * @return BAMBOO_OK if everything went fine.
 */
bamboo_error_t init_env(void) {
	bamboo_error_t err;

	/* Initialize the interpreter. */
	err = lisp_env_init(&repl_env, &cad_instance);
	IF_BAMBOO_ERROR(err)
		return err;

	/* Set our initialized flag. */
	env_initialized = true;

	/* Add our own REPL-related built-in functions. */
	/*
	err = repl_populate_builtins(&repl_env);
	IF_BAMBOO_ERROR(err)
		return err;
	*/

	return err;
}

/**
 * Destroys our current Lisp environment and quit.
 *
 * @return BAMBOO_OK if we were able to clean everything up.
 */
bamboo_error_t destroy_env(void) {
	/* Do nothing if nothing was initialized. */
	if (!env_initialized)
		return BAMBOO_OK;

	/* Destroy our environment. */
	return bamboo_destroy(&repl_env);
}

/**
 * Performs some basic cleanup before the program exits.
 */
void cleanup(void) {
	/* Clean resources from the Lisp interpreter. */
	repl_destroy();
	destroy_env();

	/* Clean resources from the CAD engine. */
	graphics_sdl_clean();
	engine_instance_free(&cad_instance);
}

/**
 * Creates a classic Read-Eval-Print-Loop.
 *
 * @param ptr Parameter passed to the thread upon creation.
 * @return Thread return value.
 */
void *repl(void *ptr) {
	bamboo_error_t err;
	int retval = 0;

	/* Initialize the REPL. */
	err = BAMBOO_OK;
	repl_init();

	/* Start the REPL loop. */
	while (!repl_readline(repl_input, REPL_INPUT_MAX_LEN)) {
		atom_t parsed;
		atom_t result;
		const char *end = repl_input;

		/* Check if all we've got was an empty line. */
		if (*repl_input == '\0')
			continue;

		/* Check if we've parsed all of the statements in the expression. */
		while (*end != '\0') {
#ifdef DEBUG
			// Check out our tokens.
			bamboo_print_tokens(end);
			printf(LINEBREAK);
#endif	/* DEBUG */

			/* Parse the user's input. */
			err = bamboo_parse_expr(end, &end, &parsed);
			IF_BAMBOO_ERROR(err) {
				uint8_t spaces;

				/* Show where the user was wrong. */
				printf("%s %s", repl_input, LINEBREAK);
				for (spaces = 0; spaces < (end - repl_input); spaces++)
					putchar(' ');
				printf("^ ");

				/* Show the error message. */
				bamboo_print_error(err);
				goto next;
			}

			/* Evaluate the parsed expression. */
			err = bamboo_eval_expr(parsed, repl_env, &result);
			IF_BAMBOO_ERROR(err) {
				/* Check if we just got a quit situation. */
				if (err == (bamboo_error_t)BAMBOO_REPL_QUIT) {
					retval = (int)result.value.integer;
					err = BAMBOO_OK;

					goto quit;
				}

				/* Explain the real issue then... */
				bamboo_print_error(err);
				goto next;
			}
		}

		/* Print the evaluated result. */
		bamboo_print_expr(result);
		printf(LINEBREAK);
	next:;
	}

quit:
	/* Return the correct code. */
	IF_BAMBOO_ERROR(err)
	exit((int)err);
	exit(retval);
}

/**
 * Loads a source file into the current environment.
 *
 * @param fname     Path to the source file to be loaded.
 * @param terminate Terminate the program after loading the source file?
 */
void load_include(const char *fname, bool terminate) {
	bamboo_error_t err;
	atom_t result;
	int retval = 0;

	/* Initialize the Lisp environment. */
	if (!env_initialized) {
		err = init_env();
		IF_BAMBOO_ERROR(err)
		goto quit;
	}

	/* Load the file. */
	err = lisp_load_source(&repl_env, fname, &result);
	IF_BAMBOO_ERROR(err) {
		/* Check if we just got a quit situation. */
		if (err == (bamboo_error_t)BAMBOO_REPL_QUIT) {
			retval = (int)result.value.integer;
			err = BAMBOO_OK;

			goto quit;
		}

		/* Explain the real issue then... */
		bamboo_print_error(err);
		fprintf(stderr, LINEBREAK);
		goto quit;
	}

	/* Print the evaluated result. */
	bamboo_print_expr(result);
	printf(LINEBREAK);

	/* Continue the program execution if we want to. */
	if (!terminate)
		return;

quit:
	/* Return the correct code. */
	IF_BAMBOO_ERROR(err)
	exit((int)err);
	exit(retval);
}

/**
 * Runs a source file and quits the application after its finished.
 *
 * @param fname Path to the source file to be executed.
 */
void run_source(const char *fname) {
	load_include(fname, true);
}

/**
 * Program's main entry point.
 *
 * @param  argc Number of command-line arguments passed to the program.
 * @param  argv Command-line arguments passed to the program.
 * @return      0 if everything went fine.
 */
void parse_args(int argc, char **argv) {
	int opt;

	while ((opt = getopt(argc, argv, "-:r:l:h")) != -1) {
		switch (opt) {
			case 'r':
			case 1:
				/* Run a script. */
				run_source(optarg);
				break;
			case 'l':
				/* Load a script into the current environment. */
				load_include(optarg, false);
				break;
			case 'h':
				/* Help */
				usage(argv[0], EXIT_SUCCESS);
				break;
			case ':':
				printf("Missing argument for %c" LINEBREAK, (char)optopt);
				usage(argv[0], EXIT_FAILURE);
				break;
			case _T('?'):
				printf("Unknown option: %c" LINEBREAK, (char)optopt);
				/* Fallthrough... */
			default:
				usage(argv[0], EXIT_FAILURE);
				break;
		}
	}
}

/**
 * Prints the usage message of the program.
 *
 * @param pname  Program name.
 * @param retval Return value to be used when exiting.
 */
void usage(const char *pname, int retval) {
	printf("Usage: %s [[-rl] source]" LINEBREAK LINEBREAK, pname);

	printf("Options:" LINEBREAK);
	printf("    -r <source>  Runs the source file and quits." LINEBREAK);
	printf("    -l <source>  Loads the source file before the REPL." LINEBREAK);
	printf("    -h           Displays this message." LINEBREAK);

	printf(LINEBREAK "Author: Nathan Campos <nathan@innoveworkshop.com>" LINEBREAK);

	exit(retval);
}

/**
 * Enables Unicode support in the console if compiled with support.
 */
void enable_unicode(void) {
#ifdef UNICODE
#ifdef _WIN32
	/* _O_WTEXT = 0x10000 */
	(void)_setmode(_fileno(stdout), 0x10000);
	(void)_setmode(_fileno(stdin), 0x10000);
#endif	/* _WIN32 */
#else
	/* Force UTF-8 on platforms other than Windows. */
	setlocale(LC_ALL, "en_US.UTF-8");
#endif	/* UNICODE */
}
