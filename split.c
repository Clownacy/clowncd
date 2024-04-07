#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <sndfile.h>

#include "clowncd.h"

typedef struct State
{
	ClownCD *cd;
	FILE *output_cue_file;
	FILE *data_file;
	SNDFILE *audio_file;
	unsigned long current_sector;
	unsigned long current_sector_audio_frame;
} State;

typedef struct Timecode
{
	unsigned char minute;
	unsigned char second;
	unsigned char frame;
} Timecode;

static Timecode SectorToTimecode(const unsigned long sector)
{
	Timecode timecode;

	timecode.frame = sector % 75;
	timecode.second = sector / 75 % 60;
	timecode.minute = sector / 75 / 60;
	/* TODO: Detect the minute value being too large? */

	return timecode;
}

static cc_bool DoTrackIndex(State* const state, const unsigned int track_number, const unsigned int index_number)
{
	const ClownCD_CueTrackType track_type = ClownCD_SeekTrackIndex(state->cd, track_number, index_number);
	const Timecode timecode = SectorToTimecode(state->current_sector);

	if (track_type == CLOWNCD_CUE_TRACK_INVALID)
		return cc_false;

	fprintf(state->output_cue_file, "    INDEX %02u %02u:%02u:%02u\n", index_number, timecode.minute, timecode.second, timecode.frame);

	switch (track_type)
	{
		case CLOWNCD_CUE_TRACK_MODE1_2048:
		case CLOWNCD_CUE_TRACK_MODE1_2352:
			for (;;)
			{
				unsigned char sector[2048];

				if (!ClownCD_ReadSector(state->cd, sector))
					break;

				if (fwrite(sector, CC_COUNT_OF(sector), 1, state->data_file) != 1)
				{
					fputs("Failed to write sector to file.\n", stderr);
					return cc_false;
				}

				++state->current_sector;
			}

			break;

		case CLOWNCD_CUE_TRACK_AUDIO:
			for (;;)
			{
				short frames[0x10][2];
				const size_t frames_read = ClownCD_ReadFrames(state->cd, &frames[0][0], CC_COUNT_OF(frames));

				if (sf_writef_short(state->audio_file, &frames[0][0], CC_COUNT_OF(frames)) != CC_COUNT_OF(frames))
				{
					fprintf(stderr, "Failed to write the correct number of frames. Error message was '%s'.\n", sf_strerror(state->audio_file));
					return cc_false;
				}

				state->current_sector_audio_frame += frames_read;
				state->current_sector += state->current_sector_audio_frame / CLOWNCD_AUDIO_FRAMES_PER_SECTOR;
				state->current_sector_audio_frame %= CLOWNCD_AUDIO_FRAMES_PER_SECTOR;

				if (frames_read != CC_COUNT_OF(frames))
				{
					if (state->current_sector_audio_frame != 0)
						fputs("Audio track does not end on a sector boundary.\n", stderr);

					break;
				}
			}

			break;

		default:
			assert(cc_false);
			return cc_false;
	}

	return cc_true;
}

static ClownCD_CueTrackType GetTrackType(ClownCD* const cd, const unsigned int track_number)
{
	const ClownCD_CueTrackType track_type = ClownCD_SeekTrackIndex(cd, track_number, 0);

	if (track_type != CLOWNCD_CUE_TRACK_INVALID)
		return track_type;

	return ClownCD_SeekTrackIndex(cd, track_number, 1);
}

static cc_bool DoTrack(ClownCD* const cd, FILE* const output_cue_file, const unsigned int track_number)
{
	const ClownCD_CueTrackType track_type = GetTrackType(cd, track_number);

	char filename[] = "Track 01.iso";
	const char *file_type_string, *track_type_string;
	State state;
	unsigned int i;

	state.cd = cd;
	state.output_cue_file = output_cue_file;

	if (track_number < 1 || track_number > 99)
		return cc_false;

	filename[6] = '0' + track_number / 10;
	filename[7] = '0' + track_number % 10;

	switch (track_type)
	{
		case CLOWNCD_CUE_TRACK_INVALID:
			return cc_false;

		case CLOWNCD_CUE_TRACK_MODE1_2048:
		case CLOWNCD_CUE_TRACK_MODE1_2352:
			/* TODO: Actually check that this is an ISO and default to '.bin' otherwise. */
			filename[ 9] = 'i';
			filename[10] = 's';
			filename[11] = 'o';

			file_type_string = "BINARY";
			track_type_string = "MODE1/2048";
			break;


		case CLOWNCD_CUE_TRACK_AUDIO:
			filename[ 9] = 'o';
			filename[10] = 'g';
			filename[11] = 'g';

			file_type_string = "WAVE";
			track_type_string = "AUDIO";
			break;

		default:
			assert(cc_false);
			return cc_false;
	}

	fprintf(output_cue_file, "FILE \"%s\" %s\n  TRACK %02u %s\n", filename, file_type_string, track_number, track_type_string);

	switch (track_type)
	{
		case CLOWNCD_CUE_TRACK_MODE1_2048:
		case CLOWNCD_CUE_TRACK_MODE1_2352:
			state.data_file = fopen(filename, "wb");

			if (state.data_file == NULL)
			{
				fprintf(stderr, "Could not open file '%s'.\n", filename);
				return cc_false;
			}

			break;

		case CLOWNCD_CUE_TRACK_AUDIO:
		{
			SF_INFO info = {0};

			info.frames = cd->track.total_frames;
			info.samplerate = 44100;
			info.channels = 2;
			info.format = SF_FORMAT_OGG | SF_FORMAT_VORBIS;

			state.audio_file = sf_open(filename, SFM_WRITE, &info);

			if (state.audio_file == NULL)
			{
				fprintf(stderr, "Could not open file '%s'. Error message was '%s'.\n", filename, sf_strerror(NULL));
				return cc_false;
			}
			else
			{
				double compression_level = 1.0;

				if (sf_command(state.audio_file, SFC_SET_COMPRESSION_LEVEL, &compression_level, sizeof(compression_level)) != SF_TRUE)
					fprintf(stderr, "Failed to set compression level. Error message was '%s'.\n", sf_strerror(state.audio_file));
			}

			break;
		}

		default:
			assert(cc_false);
			return cc_false;
	}

	state.current_sector = 0;
	state.current_sector_audio_frame = 0;

	DoTrackIndex(&state, track_number, 0);
	for (i = 1; i <= 99; ++i)
		if (!DoTrackIndex(&state, track_number, i))
			break;

	switch (track_type)
	{
		case CLOWNCD_CUE_TRACK_MODE1_2048:
		case CLOWNCD_CUE_TRACK_MODE1_2352:
			fclose(state.data_file);
			break;

		case CLOWNCD_CUE_TRACK_AUDIO:
			sf_close(state.audio_file);
			break;

		default:
			assert(cc_false);
			return cc_false;
	}

	return cc_true;
}

int main(const int argc, const char** const argv)
{
	(void)argc;
	(void)argv;

	if (argc < 2)
	{
		fputs("Input filename not specified.\n", stderr);
	}
	else
	{
		ClownCD cd = ClownCD_Open(argv[1], NULL);

		if (!ClownCD_IsOpen(&cd))
		{
			fprintf(stderr, "Could not open file '%s'.\n", argv[1]);
		}
		else
		{
			const char* const output_cue_filename = "Disc.cue";
			FILE* const output_cue_file = fopen(output_cue_filename, "w");

			if (output_cue_file == NULL)
			{
				fprintf(stderr, "Could not open file '%s'.\n", output_cue_filename);
			}
			else
			{
				unsigned int i;

				/* TODO: Tracks are allowed to begin at a higher number than 1. */
				for (i = 1; i <= 99; ++i)
					if (!DoTrack(&cd, output_cue_file, i))
						break;

				fclose(output_cue_file);
			}

			ClownCD_Close(&cd);
		}
	}

	return EXIT_SUCCESS;
}
