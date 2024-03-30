#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cue.h"
#include "file-io.h"
#include "utilities.h"

/* TODO: Disable the error messages. */

typedef struct State
{
	ClownCD_File *cue_file, *header_file;
	const char *cue_filename;
	char *track_filename;
} State;

static void Callback(void* const user_data, const char* const filename, const ClownCD_CueFileType file_type, const unsigned int track, const ClownCD_CueTrackType track_type, const unsigned int index, const unsigned long frame)
{
	State* const state = (State*)user_data;
	const unsigned long ending_frame = ClownCD_CueGetTrackEndingFrame(state->cue_file, filename, track, frame);

	unsigned int type;

	(void)index;

	if (state->track_filename == NULL)
	{
		/* TODO: Hash the filename instead of copy it. */
		state->track_filename = ClownCD_DuplicateString(filename);

		if (state->track_filename == NULL)
			fputs("Could not allocate memory to duplicate track filename.\n", stderr);
	}
	else
	{
		if (strcmp(state->track_filename, filename) != 0)
			fputs("All tracks must use the same file.\n", stderr);
	}

	if (file_type != CLOWNCD_CUE_FILE_BINARY)
		fputs("Only FILE type BINARY is supported.", stderr);

	switch (track_type)
	{
		case CLOWNCD_CUE_TRACK_MODE1_2048:
			fputs("MODE1/2048 tracks are not supported: use MODE1/2352 instead.", stderr);
			/* Fallthrough */
		case CLOWNCD_CUE_TRACK_MODE1_2352:
			type = 0;
			break;

		case CLOWNCD_CUE_TRACK_AUDIO:
			type = 1;
			break;

		default:
			fputs("Unknown track type encountered.\n", stderr);
			break;
	}

	ClownCD_WriteU16BE(state->header_file, type);
	ClownCD_WriteU32BE(state->header_file, frame);
	ClownCD_WriteU32BE(state->header_file, ending_frame == 0xFFFFFFFF ? 0xFFFFFFFF : ending_frame - frame);
}

int main(const int argc, char** const argv)
{
	if (argc < 3)
	{
		fprintf(stderr, "Usage: %s input-filename output-filename\n", argv[0]);
	}
	else
	{
		const char* const cue_filename = argv[1];
		ClownCD_File cue_file = ClownCD_FileOpen(cue_filename, CLOWNCD_RB);

		if (!ClownCD_FileIsOpen(&cue_file))
		{
			fputs("Could not open input file.\n", stderr);
		}
		else
		{
			ClownCD_File header_file = ClownCD_FileOpen(argv[2], CLOWNCD_WB);

			if (!ClownCD_FileIsOpen(&header_file))
			{
				fputs("Could not open output file.\n", stderr);
			}
			else
			{
				static const char identifier[8] = {'c', 'l', 'o', 'w', 'n', 'c', 'd', '\0'};
				State state;
				unsigned int i;

				state.cue_file = &cue_file;
				state.header_file = &header_file;
				state.cue_filename = cue_filename;
				state.track_filename = NULL;

				ClownCD_FileWrite(identifier, sizeof(identifier), 1, &header_file); /* Identifier. */
				ClownCD_WriteU16BE(&header_file, 0); /* Version. */
				ClownCD_WriteU16BE(&header_file, 0); /* Total tracks (will be filled-in later). */

				for (i = 0; ; ++i)
					if (!ClownCD_CueGetTrackIndexInfo(&cue_file, i + 1, 1, Callback, &state))
						break;

				ClownCD_FileSeek(&header_file, 8 + 2, CLOWNCD_SEEK_SET);
				ClownCD_WriteU16BE(&header_file, i); /* Total tracks. */

				ClownCD_FileClose(&header_file);
			}

			ClownCD_FileClose(&cue_file);
		}
	}

	return EXIT_SUCCESS;
}
