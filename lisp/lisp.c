/**
 * lisp/lisp.c
 * Integrates Bamboo Lisp with the CAD engine seamlessly.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#include "lisp.h"
#include <stdio.h>

#include "fileutils.h"

/**
 * Initializes the Lisp environment.
 *
 * @param env Lisp interpreter environment to be initialized.
 * @return BAMBOO_OK if everything went fine.
 */
bamboo_error_t lisp_env_init(env_t *env) {
	bamboo_error_t err;

	/* Initialize the lisp interpreter. */
	err = bamboo_init(env);
	IF_BAMBOO_ERROR(err)
		return err;

	/* TODO: Add our own built-ins. */

	return err;
}

/**
 * Loads the contents of a source file into the given environment.
 *
 * @param env    Pointer to the environment for the source to be evaluated in.
 * @param fname  Path to the file to be loaded.
 * @param result Return atom of the last evaluated expression in the source.
 *
 * @return BAMBOO_OK if everything went fine.
 */
bamboo_error_t lisp_load_source(env_t *env, const char *fname, atom_t *result) {
	bamboo_error_t err;
	atom_t parsed;
	TCHAR *contents;
	const TCHAR *end;

	/* Start from a clean slate. */
	err = BAMBOO_OK;
	*result = nil;

	/* Just remind the user of what's happening. */
	printf("Loading %s" LINEBREAK, fname);

	/* Get the file contents. */
	contents = slurp_file(fname);
	if (contents == NULL) {
		return bamboo_error(BAMBOO_ERROR_UNKNOWN,
							"Couldn't read the specified file for some reason");
	}

	/* Parse and evaluate the contents of the file. */
	end = contents;
	while (*end != _T('\0')) {
#ifdef DEBUG
		/* Check out our tokens. */
		bamboo_print_tokens(end);
		_tprintf(LINEBREAK);
#endif	/* DEBUG */

		/* Parse the expression. */
		err = bamboo_parse_expr(end, &end, &parsed);
		IF_BAMBOO_ERROR(err) {
			/* Show the error message. */
			bamboo_print_error(err);
			goto stop;
		}

		/* Deal with some special conditions from the parsing stage. */
		IF_BAMBOO_SPECIAL_COND(err) {
			switch (err) {
				case BAMBOO_EMPTY_LINE:
					/* Ignore things that are meant to be ignored. */
					end++;
					continue;
				default:
					break;
			}
		}

		/* Evaluate the parsed expression. */
		err = bamboo_eval_expr(parsed, *env, result);
		IF_BAMBOO_ERROR(err) {
			/* Check if we just got a quit situation. */
			if (err == (bamboo_error_t)BAMBOO_REPL_QUIT)
				goto stop;

			/* Explain the real issue then... */
			bamboo_print_error(err);
			goto stop;
		}
	}

stop:
	/* Clean up and return. */
	free(contents);
	return err;
}