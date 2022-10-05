/**
 * engine/commons.h
 * Common definitions of the CAD engine.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#ifndef _ENGINE_COMMONS_H
#define _ENGINE_COMMONS_H

#ifdef __cplusplus
extern “C” {
#endif

/* Library export prefix definition. */
#ifdef LIBRARY_EXPORTS
	#ifdef _WIN32
		#define SHARED_API __declspec(dllexport)
	#else
		#define SHARED_API extern
	#endif  /* _WIN32 */
#else
	#ifdef _WIN32
		#define SHARED_API __declspec(dllimport)
	#else
		#define SHARED_API extern
	#endif  /* _WIN32 */
#endif  /* ENGINE_EXPORTS */

/* Internal engine error codes. */
typedef enum {
	ENGINE_OK = 0,
	ENGINE_ERROR_ALLOC_FAIL
} engine_error_t;

#ifdef __cplusplus
}
#endif

#endif  /* _ENGINE_COMMONS_H */
