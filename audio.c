#include "audio.h"

#include <assert.h>

cc_bool ClownCD_AudioOpen(ClownCD_Audio* const audio, ClownCD_File* const file)
{
	ClownCD_AudioMetadata metadata;

	audio->format = CLOWNCD_AUDIO_INVALID;

#ifdef CLOWNCD_LIBSNDFILE
	if (ClownCD_libSndFileOpen(&audio->formats.libsndfile, file, &local_metadata))
		audio->format = CLOWNCD_AUDIO_LIBSNDFILE;
	else
#else
	if (ClownCD_FLACOpen(&audio->formats.flac, file, &metadata))
		audio->format = CLOWNCD_AUDIO_FLAC;
	else if (ClownCD_MP3Open(&audio->formats.mp3, file, &metadata))
		audio->format = CLOWNCD_AUDIO_MP3;
	else if (ClownCD_VorbisOpen(&audio->formats.vorbis, file, &metadata))
		audio->format = CLOWNCD_AUDIO_VORBIS;
	else if (ClownCD_WAVOpen(&audio->formats.wav, file, &metadata))
		audio->format = CLOWNCD_AUDIO_WAV;
#endif

	/* Verify that the audio is in a supported format. */
	return audio->format != CLOWNCD_AUDIO_INVALID && metadata.sample_rate == 44100 && metadata.total_channels == 2;
}

void ClownCD_AudioClose(ClownCD_Audio* const audio)
{
	switch (audio->format)
	{
		case CLOWNCD_AUDIO_INVALID:
			break;
#ifdef CLOWNCD_LIBSNDFILE
		case CLOWNCD_AUDIO_LIBSNDFILE:
			ClownCD_libSndFileClose(&audio->formats.libsndfile);
			break;
#else
		case CLOWNCD_AUDIO_FLAC:
			ClownCD_FLACClose(&audio->formats.flac);
			break;

		case CLOWNCD_AUDIO_MP3:
			ClownCD_MP3Close(&audio->formats.mp3);
			break;

		case CLOWNCD_AUDIO_VORBIS:
			ClownCD_VorbisClose(&audio->formats.vorbis);
			break;

		case CLOWNCD_AUDIO_WAV:
			ClownCD_WAVClose(&audio->formats.wav);
			break;
#endif
		default:
			assert(cc_false);
			break;
	}
}

cc_bool ClownCD_AudioSeek(ClownCD_Audio* const audio, const size_t frame)
{
	switch (audio->format)
	{
		case CLOWNCD_AUDIO_INVALID:
			return cc_false;
#ifdef CLOWNCD_LIBSNDFILE
		case CLOWNCD_AUDIO_LIBSNDFILE:
			return ClownCD_libSndFileSeek(&audio->formats.libsndfile, frame);
#else
		case CLOWNCD_AUDIO_FLAC:
			return ClownCD_FLACSeek(&audio->formats.flac, frame);

		case CLOWNCD_AUDIO_MP3:
			return ClownCD_MP3Seek(&audio->formats.mp3, frame);

		case CLOWNCD_AUDIO_VORBIS:
			return ClownCD_VorbisSeek(&audio->formats.vorbis, frame);

		case CLOWNCD_AUDIO_WAV:
			return ClownCD_WAVSeek(&audio->formats.wav, frame);
#endif
		default:
			assert(cc_false);
			return cc_false;
	}
}

size_t ClownCD_AudioRead(ClownCD_Audio* const audio, short* const buffer, const size_t total_frames)
{
	switch (audio->format)
	{
		case CLOWNCD_AUDIO_INVALID:
			return 0;
#ifdef CLOWNCD_LIBSNDFILE
		case CLOWNCD_AUDIO_LIBSNDFILE:
			return ClownCD_libSndFileRead(&audio->formats.libsndfile, buffer, total_frames);
#else
		case CLOWNCD_AUDIO_FLAC:
			return ClownCD_FLACRead(&audio->formats.flac, buffer, total_frames);

		case CLOWNCD_AUDIO_MP3:
			return ClownCD_MP3Read(&audio->formats.mp3, buffer, total_frames);

		case CLOWNCD_AUDIO_VORBIS:
			return ClownCD_VorbisRead(&audio->formats.vorbis, buffer, total_frames);

		case CLOWNCD_AUDIO_WAV:
			return ClownCD_WAVRead(&audio->formats.wav, buffer, total_frames);
#endif
		default:
			assert(cc_false);
			return 0;
	}
}
