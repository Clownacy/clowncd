#ifndef CLOWNCD_AUDIO_H
#define CLOWNCD_AUDIO_H

#include <stddef.h>

#include "clowncommon/clowncommon.h"

#include "file-io.h"
#include "flac.h"
#include "vorbis.h"
#include "wav.h"

typedef enum ClownCD_AudioFormat
{
	CLOWNCD_AUDIO_FLAC,
	CLOWNCD_AUDIO_VORBIS,
	CLOWNCD_AUDIO_WAV
} ClownCD_AudioFormat;

typedef struct ClownCD_Audio
{
	ClownCD_AudioFormat format;
	union
	{
		ClownCD_FLAC flac;
		ClownCD_Vorbis vorbis;
		ClownCD_WAV wav;
	} formats;
} ClownCD_Audio;

cc_bool ClownCD_AudioOpen(ClownCD_Audio *audio, ClownCD_File *file);
void ClownCD_AudioClose(ClownCD_Audio *audio);

cc_bool ClownCD_AudioSeek(ClownCD_Audio *audio, size_t frame);
size_t ClownCD_AudioRead(ClownCD_Audio *audio, short *buffer, size_t total_frames);

#endif /* CLOWNCD_AUDIO_H */
