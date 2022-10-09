/**
 * fileutils.h
 * Some utility functions to help us with files and paths.
 * 
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#ifndef _LISP_FILEUTILS_H
#define _LISP_FILEUTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bamboo.h"
#include <sys/types.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef _WIN32
	/* Defined INVALID_FILE_ATTRIBUTES for platforms that don't have it. */
	#ifndef INVALID_FILE_ATTRIBUTES
		#define INVALID_FILE_ATTRIBUTES 0xFFFFFFFF
	#endif  // INVALID_FILE_ATTRIBUTES
#endif  /* _WIN32 */

/* Checking */
bool file_exists(const char *fpath);
bool file_ext_match(const char *fpath, const char *ext);

/* Path Manipulation */
size_t cleanup_path(char *path);
char *extcat(const char *fpath, const char *ext);

/* File Content */
size_t file_contents_size(const char *fname);
char *slurp_file(const char *fname);

#ifdef __cplusplus
}
#endif

#endif	/* _LISP_FILEUTILS_H */