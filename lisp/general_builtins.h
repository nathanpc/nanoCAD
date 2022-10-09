/**
 * general_builtins.h
 * Some general built-in functions to get some more functionality to the Lisp
 * interpreter.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#ifndef _LISP_GENERAL_BUILTINS_H
#define _LISP_GENERAL_BUILTINS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bamboo.h"

bamboo_error_t builtins_populate_general(env_t *env);

#ifdef __cplusplus
}
#endif

#endif /* _LISP_GENERAL_BUILTINS_H */