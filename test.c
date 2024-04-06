#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <sndfile.h>

#include "clowncd.h"

static cc_bool DoTrack(ClownCD* const cd, FILE* const output_cue_file, const unsigned int track_number)
{
	const ClownCD_CueTrackType track_type = ClownCD_SeekTrackIndex(cd, track_number, 1);

	char filename[] = "Track 01.iso";
	const char *file_type_string, *track_type_string;

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
			/* Actually check that this is an ISO and default to '.bin' otherwise. */
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

	fprintf(output_cue_file, "FILE \"%s\" %s\n  TRACK %02u %s\n    INDEX 01 00:00:00\n", filename, file_type_string, track_number, track_type_string);

	switch (track_type)
	{
		case CLOWNCD_CUE_TRACK_MODE1_2048:
		case CLOWNCD_CUE_TRACK_MODE1_2352:
		{
			FILE* const out_file = fopen(filename, "wb");

			if (out_file == NULL)
			{
				fprintf(stderr, "Could not open file '%s'.\n", filename);
			}
			else
			{
				for (;;)
				{
					unsigned char sector[2048];

					if (!ClownCD_ReadSector(cd, sector))
						break;

					if (fwrite(sector, CC_COUNT_OF(sector), 1, out_file) != 1)
					{
						fputs("Failed to write sector to file.\n", stderr);
						break;
					}
				}

				fclose(out_file);
			}

			break;
		}

		case CLOWNCD_CUE_TRACK_AUDIO:
		{
			SF_INFO info = {0};
			SNDFILE *out_file;

			info.frames = cd->track.total_frames;
			info.samplerate = 44100;
			info.channels = 2;
			info.format = SF_FORMAT_OGG | SF_FORMAT_VORBIS;

			out_file = sf_open(filename, SFM_WRITE, &info);

			if (out_file == NULL)
			{
				fprintf(stderr, "Could not open file '%s'. Error message was '%s'.\n", filename, sf_strerror(NULL));
			}
			else
			{
				double compression_level = 1.0;

				if (sf_command(out_file, SFC_SET_COMPRESSION_LEVEL, &compression_level, sizeof(compression_level)) != SF_TRUE)
					fprintf(stderr, "Failed to set compression level. Error message was '%s'.\n", sf_strerror(out_file));

				for (;;)
				{
					short frames[0x10][2];
					const size_t frames_read = ClownCD_ReadFrames(cd, &frames[0][0], CC_COUNT_OF(frames));

					if (sf_writef_short(out_file, &frames[0][0], CC_COUNT_OF(frames)) != CC_COUNT_OF(frames))
					{
						fprintf(stderr, "Failed to write the correct number of frames. Error message was '%s'.\n", sf_strerror(out_file));
						break;
					}

					if (frames_read != CC_COUNT_OF(frames))
						break;
				}

				sf_close(out_file);
			}

			break;
		}

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
			const char* const output_cue_filename = "output.cue";
			FILE* const output_cue_file = fopen(output_cue_filename, "w");

			if (output_cue_file == NULL)
			{
				fprintf(stderr, "Could not open file '%s'.\n", output_cue_filename);
			}
			else
			{
				unsigned int i;

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
