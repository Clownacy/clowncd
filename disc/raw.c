#include "raw.h"

#include <stdlib.h>
#include <string.h>

#include "../common.h"
#include "../utilities.h"

cc_bool ClownCD_Disc_RawDetect(ClownCD_File* const file)
{
	static const unsigned char header_2352[0x10] = {0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x02, 0x00, 0x01};

	unsigned char buffer[CC_COUNT_OF(header_2352)];

	if (ClownCD_FileSeek(file, 0, CLOWNCD_SEEK_SET) != 0)
		return cc_false;

	if (ClownCD_FileRead(buffer, CC_COUNT_OF(buffer), 1, file) != 1)
		return cc_false;

	if (memcmp(buffer, header_2352, sizeof(header_2352)) != 0)
		return cc_false;

	return cc_true;
}

void ClownCD_Disc_RawOpen(ClownCD_Disc* const disc)
{
	disc->track.file = disc->file;
	disc->file = ClownCD_FileOpenBlank();
}

cc_bool ClownCD_Disc_RawSeekTrackIndex(ClownCD_Disc* const disc, const unsigned int track, const unsigned int index, const cc_bool has_full_sized_sectors)
{
	if (index != 1)
		return cc_false;

	if (track == 1)
	{
		/* Make the state file the active track file. */
		if (ClownCD_FileIsOpen(&disc->file))
		{
			disc->track.file = disc->file;
			disc->file = ClownCD_FileOpenBlank();
		}

		disc->track.audio_decoder_needed = cc_false;
		disc->track.has_full_sized_sectors = has_full_sized_sectors;
		disc->track.starting_frame = 0;
		disc->track.total_frames = -1;
	}
	else if (track <= 99 && disc->filename != NULL)
	{
		const char extensions[][4] = {
			{'F', 'L', 'A', 'C'},
			{'f', 'l', 'a', 'c'},
			{'M', 'P', '3', '\0'},
			{'m', 'p', '3', '\0'},
			{'O', 'G', 'G', '\0'},
			{'o', 'g', 'g', '\0'},
			{'W', 'A', 'V', '\0'},
			{'w', 'a', 'v', '\0'},
		};
		const char* const file_extension = ClownCD_GetFileExtension(disc->filename);
		const size_t filename_length_minus_extension = file_extension == NULL ? strlen(disc->filename) : (size_t)(file_extension - disc->filename);
		char* const audio_filename = (char*)malloc(filename_length_minus_extension + 4 + sizeof(extensions[0]) + 1);

		size_t i;

		/* Make the state file not the active track file. */
		if (!ClownCD_FileIsOpen(&disc->file))
		{
			disc->file = disc->track.file;
			disc->track.file = ClownCD_FileOpenBlank();
		}

		ClownCD_CloseTrackFile(disc);

		disc->track.audio_decoder_needed = cc_true;
		disc->track.has_full_sized_sectors = cc_true;
		disc->track.starting_frame = 0;
		disc->track.total_frames = -1;

		if (audio_filename == NULL)
			return cc_false;

		memcpy(audio_filename, disc->filename, filename_length_minus_extension);
		audio_filename[filename_length_minus_extension + 0] = ' ';
		audio_filename[filename_length_minus_extension + 1] = '0' + track / 10;
		audio_filename[filename_length_minus_extension + 2] = '0' + track % 10;
		audio_filename[filename_length_minus_extension + 3] = '.';
		audio_filename[filename_length_minus_extension + 4 + sizeof(extensions[0])] = '\0';

		for (i = 0; i < CC_COUNT_OF(extensions); ++i)
		{
			const char* const extension = extensions[i];

			memcpy(&audio_filename[filename_length_minus_extension + 4], extension, sizeof(extensions[i]));

			disc->track.file = ClownCD_FileOpen(audio_filename, CLOWNCD_RB, disc->file.functions);

			if (!ClownCD_AudioOpen(&disc->track.audio, &disc->track.file))
				ClownCD_FileClose(&disc->track.file);
			else
				break;
		}

		free(audio_filename);
	}
	else
	{
		return cc_false;
	}

	return cc_true;
}