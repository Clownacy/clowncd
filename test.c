#include <stdio.h>
#include <stdlib.h>

#include "clowncd.h"

static unsigned char sector_buffer[2352];

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

		ClownCD_Close(&cd);
	}

	return EXIT_SUCCESS;
}
