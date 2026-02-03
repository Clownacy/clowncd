#ifndef LIBCHDR_CORETYPES_H
#define LIBCHDR_CORETYPES_H

#include <stdint.h>
#include <stdio.h>

#include "../retro_inline.h"

#ifndef ARRAY_LENGTH
#define ARRAY_LENGTH(x) (sizeof(x)/sizeof(x[0]))
#endif

/*
 * arbitrary pointer to data the implementation uses to implement the below functions
 */
typedef void *chd_core_file_argp;

/*
 * return the size of a given file as a 64-bit unsigned integer.
 * the position of the file pointer after calling this function is
 * undefined because many implementations will seek to the end of the
 * file and call ftell.
 *
 * on error, (uint64_t)-1 is returned.
 */
typedef uint64_t(*chd_core_file_fsize)(void*);

/*
 * should match the behavior of fread, except the FILE* argument at the end
 * will be replaced with a struct chd_core_file*.
 */
typedef size_t(*chd_core_file_fread)(void*,size_t,size_t,void*);

/* closes the given file. */
typedef int (*chd_core_file_fclose)(void*);

/* fseek clone. */
typedef int (*chd_core_file_fseek)(void*, int64_t, int);

typedef struct chd_core_file_callbacks {
	chd_core_file_fsize fsize;
	chd_core_file_fread fread;
	chd_core_file_fclose fclose;
	chd_core_file_fseek fseek;
} core_file_callbacks;

typedef struct chd_core_file_callbacks_and_argp {
	const core_file_callbacks *callbacks;
	chd_core_file_argp argp;
} core_file_callbacks_and_argp;

/* Legacy API */

struct chd_core_file;

typedef uint64_t(*chd_core_file_fsize_legacy)(struct chd_core_file*);
typedef size_t(*chd_core_file_fread_legacy)(void*,size_t,size_t,struct chd_core_file*);
typedef int (*chd_core_file_fclose_legacy)(struct chd_core_file*);
typedef int (*chd_core_file_fseek_legacy)(struct chd_core_file*, int64_t, int);

typedef struct chd_core_file {
	chd_core_file_argp argp;
	chd_core_file_fsize_legacy fsize;
	chd_core_file_fread_legacy fread;
	chd_core_file_fclose_legacy fclose;
	chd_core_file_fseek_legacy fseek;
} core_file;

/* File IO shortcuts */

static INLINE int core_fclose(const core_file_callbacks_and_argp *fp) {
	return fp->callbacks->fclose(fp->argp);
}

static INLINE size_t core_fread(const core_file_callbacks_and_argp *fp, void *ptr, size_t len) {
	return fp->callbacks->fread(ptr, 1, len, fp->argp);
}

static INLINE int core_fseek(const core_file_callbacks_and_argp* fp, int64_t offset, int whence) {
	return fp->callbacks->fseek(fp->argp, offset, whence);
}

static INLINE uint64_t core_fsize(const core_file_callbacks_and_argp *fp)
{
	return fp->callbacks->fsize(fp->argp);
}

#endif /* LIBCHDR_CORETYPES_H */
