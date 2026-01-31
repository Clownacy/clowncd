/* license:BSD-3-Clause
 * copyright-holders:Aaron Giles
 ***************************************************************************

    libchr_zlib.h

    Zlib compression wrappers

***************************************************************************/

#ifndef LIBCHDR_ZLIB_H
#define LIBCHDR_ZLIB_H

#include <stdint.h>

#ifdef CLOWNCD_CHD_USE_SYSTEM_ZLIB
#include <zlib.h>
#else
#include "libraries/miniz-3.1.0/miniz.h"
#endif

#include "chd.h"
#include "coretypes.h"

#define MAX_ZLIB_ALLOCS				64

/* Account for an API difference between zlib and miniz. */
#ifdef CLOWNCD_CHD_USE_SYSTEM_ZLIB
typedef uInt zlib_alloc_size;
#else
typedef size_t zlib_alloc_size;
#endif

/* codec-private data for the ZLIB codec */

typedef struct _zlib_allocator zlib_allocator;
struct _zlib_allocator
{
	uint32_t *				allocptr[MAX_ZLIB_ALLOCS];
	uint32_t *				allocptr2[MAX_ZLIB_ALLOCS];
};

typedef struct _zlib_codec_data zlib_codec_data;
struct _zlib_codec_data
{
	z_stream				inflater;
	zlib_allocator			allocator;
};

/* codec-private data for the CDZL codec */
typedef struct _cdzl_codec_data cdzl_codec_data;
struct _cdzl_codec_data {
	/* internal state */
	zlib_codec_data		base_decompressor;
#ifdef WANT_SUBCODE
	zlib_codec_data		subcode_decompressor;
#endif
	uint8_t*			buffer;
};

/* zlib compression codec */
chd_error zlib_codec_init(void *codec, uint32_t hunkbytes);
void zlib_codec_free(void *codec);
chd_error zlib_codec_decompress(void *codec, const uint8_t *src, uint32_t complen, uint8_t *dest, uint32_t destlen);
voidpf zlib_fast_alloc(voidpf opaque, zlib_alloc_size items, zlib_alloc_size size);
void zlib_fast_free(voidpf opaque, voidpf address);
void zlib_allocator_free(voidpf opaque);

/* cdzl compression codec */
chd_error cdzl_codec_init(void* codec, uint32_t hunkbytes);
void cdzl_codec_free(void* codec);
chd_error cdzl_codec_decompress(void *codec, const uint8_t *src, uint32_t complen, uint8_t *dest, uint32_t destlen);

#endif /* LIBCHDR_ZLIB_H */
