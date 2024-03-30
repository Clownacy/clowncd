#include "file-io.h"

#include <assert.h>

void ClownCD_FileOpen(ClownCD_File* const file, const char* const filename, const int mode)
{
	file->stream = file->functions->open(filename, mode);
}

int ClownCD_FileClose(ClownCD_File* const file)
{
	return file->functions->close(file->stream);
}

size_t ClownCD_FileRead(void* const buffer, const size_t size, const size_t count, ClownCD_File* const file)
{
	return file->functions->read(buffer, size, count, file->stream);
}

size_t ClownCD_FileWrite(const void* const buffer, const size_t size, const size_t count, ClownCD_File* const file)
{
	return file->functions->write(buffer, size, count, file->stream);
}

long ClownCD_FileTell(ClownCD_File* const file)
{
	return file->functions->tell(file->stream);
}

int ClownCD_FileSeek(ClownCD_File* const file, const long position, const int origin)
{
	return file->functions->seek(file->stream, position, origin);
}

void WriteMemory(unsigned char* const buffer, const unsigned long value, const unsigned int total_bytes, const cc_bool big_endian)
{
	unsigned int i;

	for (i = 0; i < total_bytes; ++i)
	{
		const cc_bool i_adjusted = big_endian ? total_bytes - i - 1 : i;

		buffer[i] = (value >> (8 * i_adjusted)) & 0xFF;
	}
}

void WriteFile(ClownCD_File* const file, const unsigned long value, const unsigned int total_bytes, const cc_bool big_endian)
{
	unsigned char buffer[4];

	assert(total_bytes <= 4);
	WriteMemory(buffer, value, total_bytes, big_endian);
	ClownCD_FileWrite(buffer, total_bytes, 1, file);
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

unsigned long ReadFile(ClownCD_File* const file, const unsigned int total_bytes, const cc_bool big_endian)
{
	unsigned char buffer[4];

	assert(total_bytes <= 4);
	if (ClownCD_FileRead(buffer, total_bytes, 1, file) != 1)
		return CLOWNCD_EOF;
	return ReadMemory(buffer, total_bytes, big_endian);
}
