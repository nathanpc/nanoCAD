/**
 * input.h
 * Handles the input of expressions in the REPL.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#ifndef _CLI_INPUT_H
#define _CLI_INPUT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../lisp/lisp.h"
#include <stddef.h>

/* Initialization and Destruction */
void repl_init(void);
void repl_destroy(void);

/* Input */
int repl_readline(char *buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif  /* _CLI_INPUT_H */
