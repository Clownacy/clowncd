#include "chd.h"

#include <stdio.h>
#include <string.h>

#include "chd/streams/chd_stream.h"

#include "../common.h"

static int ClownCD_Disc_CHDFileClose(void* const stream)
{
	chdstream_close((chdstream_t*)stream);
	return 0;
}

static size_t ClownCD_Disc_CHDFileRead(void* const buffer, const size_t size, const size_t count, void* const stream)
{
	if (size == 0)
		return 0;

	return chdstream_read((chdstream_t*)stream, buffer, size * count) / size;
}

static long ClownCD_Disc_CHDFileTell(void* const stream)
{
	return chdstream_tell((chdstream_t*)stream);
}

static int ClownCD_Disc_CHDFileSeek(void* const stream, const long position, const ClownCD_FileOrigin origin)
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
	else
	{
		static const ClownCD_FileCallbacks callbacks = {
			NULL,
			ClownCD_Disc_CHDFileClose,
			ClownCD_Disc_CHDFileRead,
			NULL,
			ClownCD_Disc_CHDFileTell,
			ClownCD_Disc_CHDFileSeek,
		};

		/* TODO: Pass custom callbacks to this function so that libretro's VFS works!!! */
		chdstream_t* const chd_stream = chdstream_open(disc->filename, track);

		disc->track.file = ClownCD_FileOpenAlreadyOpen(chd_stream, &callbacks);

		disc->track.audio_decoder_needed = cc_false;
		disc->track.has_full_sized_sectors = cc_true;
		disc->track.starting_frame = 0;
		disc->track.total_frames = -1;

		return cc_true;
	}
}
