/**
 * lisp/lisp.h
 * Integrates Bamboo Lisp with the CAD engine seamlessly.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#ifndef _LISP_LISP_H
#define _LISP_LISP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>

#include "../engine/commons.h"
#include "bamboo.h"

/* Custom bamboo error codes. */
typedef enum {
	BAMBOO_REPL_QUIT = 100
} bamboo_custom_error_t;

SHARED_API bamboo_error_t lisp_env_init(env_t *env);
SHARED_API bamboo_error_t lisp_load_source(env_t *env, const char *fname,
										   atom_t *result);

#ifdef __cplusplus
}
#endif

#endif /* _LISP_LISP_H */
