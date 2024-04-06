#include "flac.h"

#include <assert.h>

#define DR_FLAC_IMPLEMENTATION
#include "libraries/dr_flac.h"

static size_t ClownCD_FLACReadCallback(void* const user_data, void* const buffer, const size_t total_bytes)
{
	ClownCD_File* const file = (ClownCD_File*)user_data;

	return ClownCD_FileRead(buffer, 1, total_bytes, file);
}

static drflac_bool32 ClownCD_FLACSeekCallback(void* const user_data, const int offset, const drflac_seek_origin origin)
{
	ClownCD_File* const file = (ClownCD_File*)user_data;

	ClownCD_FileOrigin ccd_origin;

	switch (origin)
	{
		case drflac_seek_origin_start:
			ccd_origin = CLOWNCD_SEEK_SET;
			break;

		case drflac_seek_origin_current:
			ccd_origin = CLOWNCD_SEEK_CUR;
			break;

		default:
			assert(cc_false);
			return cc_false;
	}

	return ClownCD_FileSeek(file, offset, ccd_origin) == 0;
}

cc_bool ClownCD_FLACOpen(ClownCD_FLAC* const flac, ClownCD_File* const file)
{
	flac->dr_flac = drflac_open(ClownCD_FLACReadCallback, ClownCD_FLACSeekCallback, file, NULL);
	if (flac->dr_flac != NULL)
	{
		/* Verify that the audio is in a supported format. */
		if (flac->dr_flac->channels == 2 && flac->dr_flac->sampleRate == 44100)
			return cc_true;

		drflac_close(flac->dr_flac);
	}

	return cc_false;
}

void ClownCD_FLACClose(ClownCD_FLAC* const flac)
{
	drflac_close(flac->dr_flac);
}

cc_bool ClownCD_FLACSeek(ClownCD_FLAC* const flac, const size_t frame)
{
	return drflac_seek_to_pcm_frame(flac->dr_flac, frame);
}

size_t ClownCD_FLACRead(ClownCD_FLAC* const flac, short* const buffer, const size_t total_frames)
{
	return drflac_read_pcm_frames_s16(flac->dr_flac, total_frames, buffer);
}
