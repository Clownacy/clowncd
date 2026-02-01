/* license:BSD-3-Clause
 * copyright-holders:Aaron Giles
 ***************************************************************************

    lzma.h

    LZMA compression wrappers

***************************************************************************/

#ifndef LIBCHDR_LZMA_H
#define LIBCHDR_LZMA_H

#include <stdint.h>

#include "libraries/lzma-25.01/LzmaDec.h"

#include "zlib.h"

/* codec-private data for the LZMA codec */
#define MAX_LZMA_ALLOCS 64

typedef struct _lzma_allocator lzma_allocator;
struct _lzma_allocator
{
	void *(*Alloc)(void *p, size_t size);
 	void (*Free)(void *p, void *address); /* address can be 0 */
	void (*FreeSz)(void *p, void *address, size_t size); /* address can be 0 */
	uint32_t*	allocptr[MAX_LZMA_ALLOCS];
	uint32_t*	allocptr2[MAX_LZMA_ALLOCS];
};

typedef struct _lzma_codec_data lzma_codec_data;
struct _lzma_codec_data
{
	CLzmaDec		decoder;
	lzma_allocator	allocator;
};

/* codec-private data for the CDLZ codec */
typedef struct _cdlz_codec_data cdlz_codec_data;
struct _cdlz_codec_data {
	/* internal state */
	lzma_codec_data		base_decompressor;
#ifdef WANT_SUBCODE
	zlib_codec_data		subcode_decompressor;
#endif
	uint8_t*			buffer;
};

/* lzma compression codec */
chd_error lzma_codec_init(void *codec, uint32_t hunkbytes);
void lzma_codec_free(void *codec);
chd_error lzma_codec_decompress(void *codec, const uint8_t *src, uint32_t complen, uint8_t *dest, uint32_t destlen);

/* cdlz compression codec */
chd_error cdlz_codec_init(void* codec, uint32_t hunkbytes);
void cdlz_codec_free(void* codec);
chd_error cdlz_codec_decompress(void *codec, const uint8_t *src, uint32_t complen, uint8_t *dest, uint32_t destlen);

#endif /* LIBCHDR_LZMA_H */
