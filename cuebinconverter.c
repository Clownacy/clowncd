#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void WriteLE(FILE* const file, const unsigned long value, const unsigned int total_bytes)
{
	unsigned int i;

	for (i = 0; i < total_bytes; ++i)
		fputc((value >> (8 * i)) & 0xFF, file);
}

static void Write16LE(FILE* const file, const unsigned long value)
{
	WriteLE(file, value, 2);
}

static void Write32LE(FILE* const file, const unsigned long value)
{
	WriteLE(file, value, 4);
}

static void WriteBE(FILE* const file, const unsigned long value, const unsigned int total_bytes)
{
	unsigned int i;

	for (i = 0; i < total_bytes; ++i)
		fputc((value >> (8 * (total_bytes - 1 - i))) & 0xFF, file);
}

static void Write16BE(FILE* const file, const unsigned long value)
{
	WriteBE(file, value, 2);
}

static void Write32BE(FILE* const file, const unsigned long value)
{
	WriteBE(file, value, 4);
}

int main(const int argc, char** const argv)
{
	FILE* const file = fopen(argv[1], "r");

	if (file == NULL)
	{
		fputs("Could not open input file.\n", stderr);
	}
	else
	{
/*		char filename[] = "01.wav";*/
		unsigned int i;

		fscanf(file, "CD_ROM");

		FILE* const out_file = fopen("header.bin", "wb");
		fputs("clowncd", out_file); /* Identifier. */
		fputc('\0', out_file);
		Write16BE(out_file, 0); /* Version. */
		Write16BE(out_file, 0); /* Total tracks (will be filled-in later). */

		for (i = 0; ; ++i)
		{
			char type_string[10];
			unsigned long starting_byte;
			unsigned int minute, second, frame;
			unsigned int type;

			if (fscanf(file, " // Track %*u TRACK %9s", type_string) < 1)
			{
				fputs("Could not read track header.\n", stderr);
				break;
			}

			if (strcmp(type_string, "MODE1_RAW") == 0)
			{
				starting_byte = 0;

				fscanf(file, "");
				if (fscanf(file, " NO COPY DATAFILE \"%*[^\"]\" %u:%u:%u // length in bytes: %*u", &minute, &second, &frame) < 3)
					fputs("Could not read MODE1_RAW data.\n", stderr);

				type = 0;
			}
			else if (strcmp(type_string, "AUDIO") == 0)
			{
				if (fscanf(file, " NO COPY NO PRE_EMPHASIS TWO_CHANNEL_AUDIO ZERO AUDIO %*u:%*u:%*u DATAFILE \"%*[^\"]\" #%ld %u:%u:%u // length in bytes: %*u START %*u:%*u:%*u", &starting_byte, &minute, &second, &frame) < 4)
					fputs("Could not read AUDIO data.\n", stderr);

				type = 1;
			}
			else
			{
				fprintf(stderr, "Unknown data encountered - %s.\n", type_string);
			}

			const unsigned long size = ((unsigned long)minute * 60 * 75 + (unsigned long)second * 75 + frame) * 2352;
			fprintf(stderr, "Found data starting at %lu of length %lu.\n", starting_byte / 2352, size / 2352);

			if (starting_byte % 2352 != 0)
				fprintf(stderr, "Starting byte %lu cannot be divided by 2352.\n", starting_byte);

			Write16BE(out_file, type);
			Write32BE(out_file, starting_byte / 2352);
			Write32BE(out_file, size / 2352);

			/*filename[0] = '0' + i / 10;
			filename[1] = '0' + i % 10;
			FILE* const out_file = fopen(filename, "wb");

			fputs("RIFF", out_file);
			Write32LE(out_file, size + (44 - 8));
			fputs("WAVE", out_file);
			fputs("fmt ", out_file);
			Write32LE(out_file, 16);
			Write16LE(out_file, 1);
			Write16LE(out_file, 2);
			Write32LE(out_file, 44100);
			Write32LE(out_file, 44100 * 4);
			Write16LE(out_file, 4);
			Write16LE(out_file, 16);
			fputs("data", out_file);
			Write32LE(out_file, size);

			FILE* const in_file = fopen("../../bxp/balls.bin", "rb");
			fseek(in_file, starting_byte, SEEK_SET);
			
			for (unsigned int j = 0; j < size / 2; ++j)
			{
				const int byte1 = fgetc(in_file);
				const int byte2 = fgetc(in_file);
				fputc(byte2, out_file);
				fputc(byte1, out_file);
			}
			
			fclose(in_file);

			fclose(out_file);*/
		}

		fseek(out_file, 8 + 2, SEEK_SET);
		Write16BE(out_file, i); /* Total tracks. */
		fclose(out_file);

		fclose(file);
	}

	return EXIT_SUCCESS;
}
