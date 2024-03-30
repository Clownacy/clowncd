#include <stdio.h>
#include <stdlib.h>

#include "clowncd.h"

/*static unsigned char sector_buffer[2352];*/

int main(const int argc, const char** const argv)
{
	ClownCD cd;

	(void)argc;
	(void)argv;

	if (argc < 2 || !ClownCD_OpenFromFile(&cd, argv[1]))
	{
		fputs("Couldn't open file for reading.\n", stderr);
	}
	else
	{
		unsigned int i;

		for (i = 1; ; ++i)
		{
			const ClownCD_CueTrackType track_type = ClownCD_SeekTrackIndex(&cd, i, 1);

			char filename[] = "01.wav";
			ClownCD_File out_file;

			filename[0] = '0' + i / 10;
			filename[1] = '0' + i % 10;

			if (track_type == CLOWNCD_CUE_TRACK_INVALID)
			{
				break;
			}
			else if (track_type == CLOWNCD_CUE_TRACK_MODE1_2352)
			{
				filename[3] = 'i';
				filename[4] = 's';
				filename[5] = 'o';
			}
			else if (track_type == CLOWNCD_CUE_TRACK_AUDIO)
			{
				filename[3] = 'w';
				filename[4] = 'a';
				filename[5] = 'v';
			}

			out_file = ClownCD_FileOpen(filename, CLOWNCD_WB);

			if (!ClownCD_FileIsOpen(&out_file))
			{
				fprintf(stderr, "Could not open file '%s'.\n", filename);
			}
			else
			{
				if (track_type == CLOWNCD_CUE_TRACK_MODE1_2352)
				{
					for (;;)
					{
						unsigned char sector[2048];

						if (!ClownCD_ReadSectorData(&cd, sector))
							break;

						ClownCD_FileWrite(sector, 1, CC_COUNT_OF(sector), &out_file);
					}
				}
				else if (track_type == CLOWNCD_CUE_TRACK_AUDIO)
				{
					const size_t size = cd.track.remaining_frames * 4;

					ClownCD_FileWrite("RIFF", 4, 1, &out_file);
					ClownCD_WriteU32LE(&out_file, size + (44 - 8));
					ClownCD_FileWrite("WAVE", 4, 1, &out_file);
					ClownCD_FileWrite("fmt ", 4, 1, &out_file);
					ClownCD_WriteU32LE(&out_file, 16);
					ClownCD_WriteU16LE(&out_file, 1);
					ClownCD_WriteU16LE(&out_file, 2);
					ClownCD_WriteU32LE(&out_file, 44100);
					ClownCD_WriteU32LE(&out_file, 44100 * 4);
					ClownCD_WriteU16LE(&out_file, 4);
					ClownCD_WriteU16LE(&out_file, 16);
					ClownCD_FileWrite("data", 4, 1, &out_file);
					ClownCD_WriteU32LE(&out_file, size);

					for (;;)
					{
						short frames[0x10][2];
						const size_t frames_read = ClownCD_ReadAudioFrames(&cd, &frames[0][0], CC_COUNT_OF(frames));

						size_t j;

						for (j = 0; j < frames_read; ++j)
						{
							ClownCD_WriteS16LE(&out_file, frames[j][0]);
							ClownCD_WriteS16LE(&out_file, frames[j][1]);
						}

						if (frames_read != CC_COUNT_OF(frames))
							break;
					}
				}

				ClownCD_FileClose(&out_file);
			}
		}
#if 0
		FILE *file;

		if (ClownCD_SeekTrackIndex(&cd, 1, 1) == CLOWNCD_CUE_TRACK_INVALID)
			fputs("Couldn't seek to track 1 index 1.\n", stderr);

		if (!ClownCD_ReadSectorAtRaw(&cd, 0, sector_buffer))
			fputs("Couldn't read sector 0.\n", stderr);

		file = fopen("out.bin", "wb");

		if (file == NULL)
		{
			fputs("Couldn't open file for writing.\n", stderr);
		}
		else
		{
			fwrite(sector_buffer, 2352, 1, file);
			fclose(file);

			fprintf(stderr, "CRC is %scorrect\n", ClownCD_ValidateSectorCRC(sector_buffer) ? "" : "in");
		}
#endif

		ClownCD_Close(&cd);
	}

	return EXIT_SUCCESS;
}
