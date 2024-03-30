#include "file-io.h"

#include <assert.h>

void WriteMemory(unsigned char* const buffer, const unsigned long value, const unsigned int total_bytes, const cc_bool big_endian)
{
	unsigned int i;

	for (i = 0; i < total_bytes; ++i)
	{
		const cc_bool i_adjusted = big_endian ? total_bytes - i - 1 : i;

		buffer[i] = (value >> (8 * i_adjusted)) & 0xFF;
	}
}

void WriteFile(FILE* const file, const unsigned long value, const unsigned int total_bytes, const cc_bool big_endian)
{
	unsigned char buffer[4];

	assert(total_bytes <= 4);
	WriteMemory(buffer, value, total_bytes, big_endian);
	fwrite(buffer, total_bytes, 1, file);
}

unsigned long ReadMemory(const unsigned char* const buffer, const unsigned int total_bytes, const cc_bool big_endian)
{
	unsigned long value = 0;
	unsigned int i;

	for (i = 0; i < total_bytes; ++i)
	{
		const cc_bool i_adjusted = big_endian ? total_bytes - i - 1 : i;

		value <<= 8;
		value |= buffer[i_adjusted];
	}

	return value;
}

unsigned long ReadFile(FILE* const file, const unsigned int total_bytes, const cc_bool big_endian)
{
	unsigned char buffer[4];

	assert(total_bytes <= 4);
	if (fread(buffer, total_bytes, 1, file) != 1)
		return 0;
	return ReadMemory(buffer, total_bytes, big_endian);
}
