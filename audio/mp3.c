#include "mp3.h"

#include <assert.h>

#define DR_MP3_IMPLEMENTATION
#include "libraries/dr_mp3.h"

static size_t ClownCD_MP3ReadCallback(void* const user_data, void* const buffer, const size_t total_bytes)
{
	ClownCD_File* const file = (ClownCD_File*)user_data;

	return ClownCD_FileRead(buffer, 1, total_bytes, file);
}

static drmp3_bool32 ClownCD_MP3SeekCallback(void* const user_data, const int offset, const drmp3_seek_origin origin)
{
	ClownCD_File* const file = (ClownCD_File*)user_data;

	ClownCD_FileOrigin ccd_origin;

	switch (origin)
	{
		case drmp3_seek_origin_start:
			ccd_origin = CLOWNCD_SEEK_SET;
			break;

		case drmp3_seek_origin_current:
			ccd_origin = CLOWNCD_SEEK_CUR;
			break;

		default:
			assert(cc_false);
			return cc_false;
	}

	return ClownCD_FileSeek(file, offset, ccd_origin) == 0;
}

cc_bool ClownCD_MP3Open(ClownCD_MP3* const mp3, ClownCD_File* const file, ClownCD_AudioMetadata* const metadata)
{
	if (ClownCD_FileSeek(file, 0, CLOWNCD_SEEK_SET) != 0)
		return cc_false;

	if (drmp3_init(&mp3->dr_mp3, ClownCD_MP3ReadCallback, ClownCD_MP3SeekCallback, file, NULL))
	{
		metadata->sample_rate = mp3->dr_mp3.sampleRate;
		metadata->total_channels = mp3->dr_mp3.channels;

		return cc_true;
	}

	return cc_false;
}

void ClownCD_MP3Close(ClownCD_MP3* const mp3)
{
	drmp3_uninit(&mp3->dr_mp3);
}

cc_bool ClownCD_MP3Seek(ClownCD_MP3* const mp3, const size_t frame)
{
	return drmp3_seek_to_pcm_frame(&mp3->dr_mp3, frame);
}

size_t ClownCD_MP3Read(ClownCD_MP3* const mp3, short* const buffer, const size_t total_frames)
{
	return drmp3_read_pcm_frames_s16(&mp3->dr_mp3, total_frames, buffer);
}
