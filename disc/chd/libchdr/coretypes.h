#ifndef LIBCHDR_CORETYPES_H
#define LIBCHDR_CORETYPES_H

#include <stdint.h>
#include <stdio.h>

#include "../retro_inline.h"

#ifndef ARRAY_LENGTH
#define ARRAY_LENGTH(x) (sizeof(x)/sizeof(x[0]))
#endif

typedef struct chd_core_file_callbacks {
	/*
	 * return the size of a given file as a 64-bit unsigned integer.
	 * the position of the file pointer after calling this function is
	 * undefined because many implementations will seek to the end of the
	 * file and call ftell.
	 *
	 * on error, (uint64_t)-1 is returned.
	 */
	uint64_t(*fsize)(void*);

	/*
	 * should match the behavior of fread, except the FILE* argument at the end
	 * will be replaced with a struct chd_core_file*.
	 */
	size_t(*fread)(void*,size_t,size_t,void*);

	/* closes the given file. */
	int (*fclose)(void*);

	/* fseek clone. */
	int (*fseek)(void*, int64_t, int);
} core_file_callbacks;

typedef struct chd_core_file {
	const core_file_callbacks *callbacks;

	/*
	 * arbitrary pointer to data the implementation uses to implement the above functions
	 */
	void *argp;
} core_file;

static INLINE int core_fclose(core_file *fp) {
	return fp->callbacks->fclose(fp->argp);
}

static INLINE size_t core_fread(core_file *fp, void *ptr, size_t len) {
	return fp->callbacks->fread(ptr, 1, len, fp->argp);
}

static INLINE int core_fseek(core_file* fp, int64_t offset, int whence) {
	return fp->callbacks->fseek(fp->argp, offset, whence);
}

static INLINE uint64_t core_fsize(core_file *fp)
{
	return fp->callbacks->fsize(fp->argp);
}

#endif /* LIBCHDR_CORETYPES_H */
