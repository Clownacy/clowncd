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

		ClownCD_ReadSectorRaw(&cd, 0, sector_buffer);

		file = fopen("out.bin", "wb");

		if (file == NULL)
		{
			fputs("Couldn't open file for writing.\n", stderr);
		}
		else
		{
			fwrite(sector_buffer, 0x800 + 0x10, 1, file);
			fclose(file);

			fprintf(stderr, "CRC is %scorrect\n", ClownCD_ValidateSectorCRC(sector_buffer) ? "" : "in");
		}

		ClownCD_Close(&cd);
	}

	return EXIT_SUCCESS;
}
