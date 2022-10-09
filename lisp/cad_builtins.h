/**
 * cad_builtins.c
 * Some CAD built-in functions to integrate the Lisp interpreter and the CAD
 * engine.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#ifndef _LISP_CAD_BUILTINS_H
#define _LISP_CAD_BUILTINS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../engine/engine.h"
#include "bamboo.h"

bamboo_error_t builtins_setup_cad(env_t *env, instance_t *instance);

#ifdef __cplusplus
}
#endif

#endif /* _LISP_CAD_BUILTINS_H */