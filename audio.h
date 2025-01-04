#ifndef CLOWNCD_AUDIO_H
#define CLOWNCD_AUDIO_H

#include <stddef.h>

#include "clowncommon/clowncommon.h"

#define CLOWNRESAMPLER_STATIC
#define CLOWNRESAMPLER_NO_HIGH_LEVEL_ADJUST
#define CLOWNRESAMPLER_NO_HIGH_LEVEL_RESAMPLE_END
#define CLOWNRESAMPLER_MAXIMUM_CHANNELS 2 /* We only need stereo. */
#include "audio/libraries/clownresampler/clownresampler.h"

#ifdef CLOWNCD_LIBSNDFILE
 #include "audio/libsndfile.h"
#else
 #include "audio/flac.h"
 #include "audio/mp3.h"
 #include "audio/vorbis.h"
 #include "audio/wav.h"
#endif
#include "file-io.h"

typedef enum ClownCD_AudioFormat
{
	CLOWNCD_AUDIO_INVALID,
#ifdef CLOWNCD_LIBSNDFILE
	CLOWNCD_AUDIO_LIBSNDFILE
#else
	CLOWNCD_AUDIO_FLAC,
	CLOWNCD_AUDIO_MP3,
	CLOWNCD_AUDIO_VORBIS,
	CLOWNCD_AUDIO_WAV
#endif
} ClownCD_AudioFormat;

typedef struct ClownCD_Audio
{
	ClownCD_AudioFormat format;
	union
	{
#ifdef CLOWNCD_LIBSNDFILE
		ClownCD_libSndFile libsndfile;
#else
		ClownCD_FLAC flac;
		ClownCD_MP3 mp3;
		ClownCD_Vorbis vorbis;
		ClownCD_WAV wav;
#endif
	} formats;
	ClownResampler_HighLevel_State resampler;
} ClownCD_Audio;

cc_bool ClownCD_AudioOpen(ClownCD_Audio *audio, ClownCD_File *file);
void ClownCD_AudioClose(ClownCD_Audio *audio);

cc_bool ClownCD_AudioSeek(ClownCD_Audio *audio, size_t frame);
size_t ClownCD_AudioRead(ClownCD_Audio *audio, short *buffer, size_t total_frames);

#endif /* CLOWNCD_AUDIO_H */
