/* license:BSD-3-Clause
 * copyright-holders:Aaron Giles
 ***************************************************************************

    libchr_zstd.h

    Zstd compression wrappers

***************************************************************************/

#ifndef LIBCHDR_ZSTD_H
#define LIBCHDR_ZSTD_H

#include <stdint.h>

#ifdef CLOWNCD_CHD_USE_SYSTEM_ZSTD
#include <zstd.h>
#else
#include "libraries/zstd-1.5.7/zstd.h"
#endif

#include "chd.h"
#include "coretypes.h"

typedef struct _zstd_codec_data zstd_codec_data;
struct _zstd_codec_data
{
	ZSTD_DStream *dstream;
};

typedef struct _cdzs_codec_data cdzs_codec_data;
struct _cdzs_codec_data
{
	zstd_codec_data base_decompressor;
#ifdef WANT_SUBCODE
	zstd_codec_data subcode_decompressor;
#endif
	uint8_t*				buffer;
};

#endif /* LIBCHDR_ZSTD_H */
