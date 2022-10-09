/**
 * fileutils.c
 * Some utility functions to help us with files and paths.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#include "fileutils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
	#include <unistd.h>
#endif  /* !_WIN32 */

 /**
  * Checks if a file exists.
  *
  * @param fpath File path to be checked.
  * @return TRUE if the file exists.
  */
bool file_exists(const char *fpath) {
#ifdef _WIN32
	DWORD dwAttrib;

	/* Get file attributes and return. */
	dwAttrib = GetFileAttributes(fpath);
	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#else
	return access(fpath, F_OK) != -1;
#endif  /* _WIN32 */
}

/**
 * Checks if a file extension is the same as the one specified.
 *
 * @param fpath File path to be checked.
 * @param ext   Desired file extension.
 *
 * @return TRUE if the file has the desired extension.
 */
bool file_ext_match(const char *fpath, const char *ext) {
	const char *fext;
	int i;

	/* Go through the file path backwards trying to find a dot. */
	fext = fpath;
	for (i = (strlen(fpath) - 1); i >= 0; i--) {
		if (fpath[i] == '.') {
			fext = fpath + i + 1;
			break;
		}
	}

	return strcmp(fext, ext) == 0;
}

/**
 * Cleans up a path string and converts it to Windows separators if necessary.
 *
 * @param path File path string.
 * @return Final size of the cleaned up path.
 */
size_t cleanup_path(char *path) {
	char *pos;
	char *back;

	/* Go through string searching for duplicate slashes. */
	while ((pos = strstr(path, "//")) != NULL) {
		/* Append the rest of the string skipping one character. */
		for (back = pos++; *back != '\0'; back++) {
			*back = *pos++;
		}
	}

#ifdef _WIN32
	/* Fix strings that have a mix of Windows and UNIX slashes together. */
	while ((pos = strstr(path, "\\/")) != NULL) {
		/* Append the rest of the string skipping one character. */
		for (back = pos++; *back != '\0'; back++) {
			*back = *pos++;
		}
	}

	/* Convert UNIX path separators to Windows. */
	pos = path;
	for (; *pos != '\0'; pos++) {
		if (*pos == '/')
			*pos = '\\';
	}
#endif  /* _WIN32 */

	return strlen(path);
}

/**
 * Concatenates an extension to a file path.
 * WARNING: This function allocates its return string, so you're responsible
 *          for freeing it.
 *
 * @param fpath Path to the file without extension.
 * @param ext   File extension without the dot.
 *
 * @return Path to the file with extension appended.
 */
char* extcat(const char *fpath, const char *ext) {
	char *final_path;

	/* Allocate the final path string. */
	final_path = (char *)malloc((strlen(fpath) + strlen(ext) + 2) *
		sizeof(char));
	if (final_path == NULL)
		return NULL;

	/* Concatenate things. */
	sprintf(final_path, "%s." SPEC_STR, fpath, ext);

	return final_path;
}

/**
 * Gets the size of a buffer to hold the whole contents of a file.
 *
 * @param fname File path.
 * @return Size of the content of specified file or 0 if an error occurred.
 */
size_t file_contents_size(const char *fname) {
	FILE *fh;
	size_t nbytes;

	/* Open file. */
	fh = fopen(fname, "r");
	if (fh == NULL)
		return 0L;

	/* Seek file to determine its size. */
	fseek(fh, 0L, SEEK_END);
	nbytes = ftell(fh);

	/* Close the file handler and return. */
	fclose(fh);
	return nbytes;
}

/**
 * Reads a whole file and stores it into a string.
 * WARNING: Remember to free the returned string allocated by this function.
 *
 * @param fname File path.
 * @return String where the file contents are to be stored (will be allocated by
 *         this function).
 */
char* slurp_file(const char *fname) {
	FILE *fh;
	size_t fsize;
	int c;
	char *contents;
	char *tmp;

	/* Get file size. */
	fsize = file_contents_size(fname);
	if (fsize == 0L)
		return NULL;

	/* Allocate the string to hold the contents of the file. */
	contents = (char *)malloc((fsize + 1) * sizeof(char));
	if (contents == NULL)
		return NULL;

	/* Open file to read its contents. */
	fh = fopen(fname, "r");
	if (fh == NULL) {
		free(contents);
		return NULL;
	}

	/* Reads the whole file into the buffer. */
	tmp = contents;
	c = fgetc(fh);
	while (c != EOF) {
		/* Append the character to our contents buffer and advance it. */
		*tmp = (char)c;
		tmp++;

		/* Get a character from the file. */
		c = fgetc(fh);
	}

	/* Close the file handle and make sure our string is properly terminated. */
	fclose(fh);
	*tmp = '\0';

	return contents;
}
