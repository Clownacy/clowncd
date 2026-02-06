#include "chd.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chd/streams/chd_stream.h"

#include "../common.h"

/* File IO functions for reading from CHD track streams. */

static int ClownCD_Disc_CHDTrackStreamClose(void* const stream)
{
	chdstream_close((chdstream_t*)stream);
	return 0;
}

static size_t ClownCD_Disc_CHDTrackStreamRead(void* const buffer, const size_t size, const size_t count, void* const stream)
{
	if (size == 0)
		return 0;

	return chdstream_read((chdstream_t*)stream, buffer, size * count) / size;
}

static long ClownCD_Disc_CHDTrackStreamTell(void* const stream)
{
	return chdstream_tell((chdstream_t*)stream);
}

static int ClownCD_Disc_CHDTrackStreamSeek(void* const stream, const long position, const ClownCD_FileOrigin origin)
{
	switch (origin)
	{
		case CLOWNCD_SEEK_SET:
			return chdstream_seek((chdstream_t*)stream, position, SEEK_SET);

		case CLOWNCD_SEEK_CUR:
			return chdstream_seek((chdstream_t*)stream, position, SEEK_CUR);

		case CLOWNCD_SEEK_END:
			return chdstream_seek((chdstream_t*)stream, position, SEEK_END);
	}

	return -1;
}

/* File IO functions for reading from CHD files. */

uint64_t ClownCD_Disc_CHDFileSize(void* const user_data)
{
	ClownCD_File* const file = (ClownCD_File*)user_data;

	return ClownCD_FileSize(file);
}

size_t ClownCD_Disc_CHDFileRead(void* const buffer , const size_t size, const size_t count, void* const user_data)
{
	ClownCD_File* const file = (ClownCD_File*)user_data;

	return ClownCD_FileRead(buffer, size, count, file);
}

int ClownCD_Disc_CHDFileClose(void* const user_data)
{
	(void)user_data;
	return 0;
}

int ClownCD_Disc_CHDFileSeek(void* const user_data, const int64_t offset, const int raw_origin)
{
	ClownCD_File* const file = (ClownCD_File*)user_data;

	ClownCD_FileOrigin origin;

	/* TODO: 64-bit file IO support? */
	if (offset < LONG_MIN || offset > LONG_MAX)
		return -1;

	switch (raw_origin)
	{
		case SEEK_SET:
			origin = CLOWNCD_SEEK_SET;
			break;

		case SEEK_CUR:
			origin = CLOWNCD_SEEK_CUR;
			break;

		case SEEK_END:
			origin = CLOWNCD_SEEK_END;
			break;

		default:
			return -1;
	}

	return ClownCD_FileSeek(file, offset, origin);
}

/* Disc API. */

cc_bool ClownCD_Disc_CHDDetect(ClownCD_File* const file)
{
	static const unsigned char magic[8] = {'M', 'C', 'o', 'm', 'p', 'r', 'H', 'D'};

	unsigned char buffer[CC_COUNT_OF(magic)];

	if (ClownCD_FileSeek(file, 0, CLOWNCD_SEEK_SET) != 0)
		return cc_false;

	if (ClownCD_FileRead(buffer, CC_COUNT_OF(buffer), 1, file) != 1)
		return cc_false;

	if (memcmp(buffer, magic, sizeof(magic)) != 0)
		return cc_false;

	return cc_true;
}

void ClownCD_Disc_CHDOpen(ClownCD_Disc* const disc)
{
	disc->track.file = ClownCD_FileOpenBlank();
}

cc_bool ClownCD_Disc_CHDSeekTrackIndex(ClownCD_Disc* const disc, const unsigned int track, const unsigned int index)
{
	if (index != 1)
	{
		return cc_false;
	}
	else if (ClownCD_FileClose(&disc->track.file) != 0)
	{
		return cc_false;
	}
	else
	{
		static const core_file_callbacks file_callbacks = {
			ClownCD_Disc_CHDFileSize,
			ClownCD_Disc_CHDFileRead,
			ClownCD_Disc_CHDFileClose,
			ClownCD_Disc_CHDFileSeek,
		};

		static const ClownCD_FileCallbacks track_callbacks = {
			NULL,
			ClownCD_Disc_CHDTrackStreamClose,
			ClownCD_Disc_CHDTrackStreamRead,
			NULL,
			ClownCD_Disc_CHDTrackStreamTell,
			ClownCD_Disc_CHDTrackStreamSeek,
		};

		chdstream_t* const chd_stream = chdstream_open_core_file_callbacks(&file_callbacks, &disc->file, track);

		disc->track.file = ClownCD_FileOpenAlreadyOpen(chd_stream, &track_callbacks);

		disc->track.audio_decoder_needed = cc_false;
		disc->track.has_full_sized_sectors = cc_true;
		disc->track.starting_frame = 0;
		disc->track.total_frames = -1;

		return cc_true;
	}
}
