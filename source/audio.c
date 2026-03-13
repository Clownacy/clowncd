#include "audio.h"

#include <assert.h>

#define CLOWNCD_AUDIO_SAMPLE_RATE 44100
#define CLOWNCD_AUDIO_TOTAL_CHANNELS 2

/* TODO: Move this to a user-provided buffer, in case the user doesn't want to always keep this allocated? */
static ClownResampler_Precomputed clowncd_precomputed;
static cc_bool clowncd_precomputed_done;

cc_bool ClownCD_AudioOpen(ClownCD_Audio* const audio, ClownCD_File* const file)
{
	ClownCD_AudioMetadata metadata;

	audio->format = CLOWNCD_AUDIO_INVALID;

#ifdef CLOWNCD_LIBSNDFILE
	if (ClownCD_libSndFileOpen(&audio->formats.libsndfile, file, &metadata))
		audio->format = CLOWNCD_AUDIO_LIBSNDFILE;
	else
#else
	/* Always test MP3 last since it is not easy to detect MP3 files automatically,
	   so only use it as a last resort. */
	if (ClownCD_FLACOpen(&audio->formats.flac, file, &metadata))
		audio->format = CLOWNCD_AUDIO_FLAC;
	else if (ClownCD_VorbisOpen(&audio->formats.vorbis, file, &metadata))
		audio->format = CLOWNCD_AUDIO_VORBIS;
	else if (ClownCD_WAVOpen(&audio->formats.wav, file, &metadata))
		audio->format = CLOWNCD_AUDIO_WAV;
	else if (ClownCD_MP3Open(&audio->formats.mp3, file, &metadata))
		audio->format = CLOWNCD_AUDIO_MP3;
#endif

	/* Verify that the audio is in a supported format. */
	/* TODO: Support mono audio! */
	if (audio->format == CLOWNCD_AUDIO_INVALID || (metadata.total_channels != 1 && metadata.total_channels != 2))
		return cc_false;

	if (!clowncd_precomputed_done)
	{
		clowncd_precomputed_done = cc_true;
		ClownResampler_Precompute(&clowncd_precomputed);
	}

	/* Resample to the native CD sample rate. */
	ClownResampler_HighLevel_Init(&audio->resampler, metadata.total_channels, metadata.sample_rate, CLOWNCD_AUDIO_SAMPLE_RATE, CLOWNCD_AUDIO_SAMPLE_RATE);

	return cc_true;
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
	/* Cheeky hack to account for resampling. */
	/* This is a fixed-point multiplication that splits the multiplicand to avoid overflow. */
	const size_t corrected_frame_upper = audio->resampler.low_level.increment * (frame / CLOWNRESAMPLER_FIXED_POINT_FRACTIONAL_SIZE);
	const size_t corrected_frame_lower = (audio->resampler.low_level.increment * (frame % CLOWNRESAMPLER_FIXED_POINT_FRACTIONAL_SIZE)) / CLOWNRESAMPLER_FIXED_POINT_FRACTIONAL_SIZE;
	const size_t corrected_frame = corrected_frame_upper + corrected_frame_lower;

	switch (audio->format)
	{
		case CLOWNCD_AUDIO_INVALID:
			return cc_false;
#ifdef CLOWNCD_LIBSNDFILE
		case CLOWNCD_AUDIO_LIBSNDFILE:
			return ClownCD_libSndFileSeek(&audio->formats.libsndfile, corrected_frame);
#else
		case CLOWNCD_AUDIO_FLAC:
			return ClownCD_FLACSeek(&audio->formats.flac, corrected_frame);

		case CLOWNCD_AUDIO_MP3:
			return ClownCD_MP3Seek(&audio->formats.mp3, corrected_frame);

		case CLOWNCD_AUDIO_VORBIS:
			return ClownCD_VorbisSeek(&audio->formats.vorbis, corrected_frame);

		case CLOWNCD_AUDIO_WAV:
			return ClownCD_WAVSeek(&audio->formats.wav, corrected_frame);
#endif
		default:
			assert(cc_false);
			return cc_false;
	}
}

typedef struct ClownCD_ResamplerCallbackData
{
	ClownCD_Audio *audio;
	short *output_pointer;
	size_t output_buffer_frames_remaining;
} ClownCD_ResamplerCallbackData;

static size_t ClownCD_ResamplerInputCallback(void* const user_data, cc_s16l* const buffer, const size_t total_frames)
{
	const ClownCD_ResamplerCallbackData* const callback_data = (const ClownCD_ResamplerCallbackData*)user_data;
	ClownCD_Audio* const audio = callback_data->audio;

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

static cc_bool ClownCD_ResamplerOutputCallback(void* const user_data, const cc_s32f* const frame, const cc_u8f total_samples)
{
	ClownCD_ResamplerCallbackData* const callback_data = (ClownCD_ResamplerCallbackData*)user_data;

	cc_u8f i;

	/* Output the frame. */
	for (i = 0; i < total_samples; ++i)
	{
		cc_s32f sample;

		sample = frame[i];

		/* Clamp the sample to 16-bit. */
		if (sample > 0x7FFF)
			sample = 0x7FFF;
		else if (sample < -0x7FFF)
			sample = -0x7FFF;

		/* Push the sample to the output buffer. */
		callback_data->output_pointer[i] = (short)sample;
	}

	/* Upsample mono to stereo. */
	if (total_samples == 1)
		callback_data->output_pointer[1] = callback_data->output_pointer[0];

	callback_data->output_pointer += CLOWNCD_AUDIO_TOTAL_CHANNELS;

	/* Signal whether there is more room in the output buffer. */
	return --callback_data->output_buffer_frames_remaining != 0;
}

size_t ClownCD_AudioRead(ClownCD_Audio* const audio, short* const buffer, const size_t total_frames)
{
	ClownCD_ResamplerCallbackData callback_data;

	callback_data.audio = audio;
	callback_data.output_pointer = buffer;
	callback_data.output_buffer_frames_remaining = total_frames;

	/* Resample the decoded audio data. */
	ClownResampler_HighLevel_Resample(&audio->resampler, &clowncd_precomputed, ClownCD_ResamplerInputCallback, ClownCD_ResamplerOutputCallback, &callback_data);

	return total_frames - callback_data.output_buffer_frames_remaining;
}
