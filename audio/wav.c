#include "wav.h"

#include <assert.h>

#define DR_WAV_IMPLEMENTATION
#include "libraries/dr_wav.h"

static size_t ClownCD_WAVReadCallback(void* const user_data, void* const buffer, const size_t total_bytes)
{
	ClownCD_File* const file = (ClownCD_File*)user_data;

	return ClownCD_FileRead(buffer, 1, total_bytes, file);
}

static drwav_bool32 ClownCD_WAVSeekCallback(void* const user_data, const int offset, const drwav_seek_origin origin)
{
	ClownCD_File* const file = (ClownCD_File*)user_data;

	ClownCD_FileOrigin ccd_origin;

	switch (origin)
	{
		case DRWAV_SEEK_SET:
			ccd_origin = CLOWNCD_SEEK_SET;
			break;

		case DRWAV_SEEK_CUR:
			ccd_origin = CLOWNCD_SEEK_CUR;
			break;

		default:
			assert(cc_false);
			return cc_false;
	}

	return ClownCD_FileSeek(file, offset, ccd_origin) == 0;
}

static drwav_bool32 ClownCD_WAVTellCallback(void* const user_data, drwav_int64* const cursor)
{
	ClownCD_File* const file = (ClownCD_File*)user_data;

	*cursor = ClownCD_FileTell(file);
	return DRWAV_TRUE;
}

cc_bool ClownCD_WAVOpen(ClownCD_WAV* const wav, ClownCD_File* const file, ClownCD_AudioMetadata* const metadata)
{
	if (ClownCD_FileSeek(file, 0, CLOWNCD_SEEK_SET) != 0)
		return cc_false;

	if (drwav_init(&wav->dr_wav, ClownCD_WAVReadCallback, ClownCD_WAVSeekCallback, ClownCD_WAVTellCallback, file, NULL))
	{
		metadata->sample_rate = wav->dr_wav.sampleRate;
		metadata->total_channels = wav->dr_wav.channels;

		return cc_true;
	}

	return cc_false;
}

void ClownCD_WAVClose(ClownCD_WAV* const wav)
{
	drwav_uninit(&wav->dr_wav);
}

cc_bool ClownCD_WAVSeek(ClownCD_WAV* const wav, const size_t frame)
{
	return drwav_seek_to_pcm_frame(&wav->dr_wav, frame);
}

size_t ClownCD_WAVRead(ClownCD_WAV* const wav, short* const buffer, const size_t total_frames)
{
	return drwav_read_pcm_frames_s16(&wav->dr_wav, total_frames, buffer);
}
