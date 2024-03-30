#include "file-io.h"

#include <assert.h>

#include <stdio.h>

static void* ClownCD_FileOpenStandard(const char* const filename, const ClownCD_FileMode mode)
{
	const char *standard_mode;

	switch (mode)
	{
		case CLOWNCD_MODE_READ:
			standard_mode = "rb";
			break;

		case CLOWNCD_MODE_WRITE:
			standard_mode = "wb";
			break;

		default:
			return NULL;
	}

	return fopen(filename, standard_mode);
}

static int ClownCD_FileCloseStandard(void* const stream)
{
	return fclose((FILE*)stream);
}

static size_t ClownCD_FileReadStandard(void* const buffer, const size_t size, const size_t count, void* const stream)
{
	return fread(buffer, size, count, (FILE*)stream);
}

static size_t ClownCD_FileWriteStandard(const void* const buffer, const size_t size, const size_t count, void* const stream)
{
	return fwrite(buffer, size, count, (FILE*)stream);
}

static long ClownCD_FileTellStandard(void* const stream)
{
	return ftell((FILE*)stream);
}

static int ClownCD_FileSeekStandard(void* const stream, const long position, const ClownCD_FileOrigin origin)
{
	int standard_origin;

	switch (origin)
	{
		case CLOWNCD_SEEK_SET:
			standard_origin = SEEK_SET;
			break;

		case CLOWNCD_SEEK_CUR:
			standard_origin = SEEK_CUR;
			break;

		case CLOWNCD_SEEK_END:
			standard_origin = SEEK_END;
			break;

		default:
			return 1;
	}

	return fseek((FILE*)stream, position, standard_origin);
}

ClownCD_File ClownCD_FileOpen(const char* const filename, const ClownCD_FileMode mode)
{
	static const ClownCD_FileCallbacks callbacks = {
		ClownCD_FileOpenStandard,
		ClownCD_FileCloseStandard,
		ClownCD_FileReadStandard,
		ClownCD_FileWriteStandard,
		ClownCD_FileTellStandard,
		ClownCD_FileSeekStandard
	};

	return ClownCD_FileOpenCustomIO(&callbacks, filename, mode);
}

ClownCD_File ClownCD_FileOpenCustomIO(const ClownCD_FileCallbacks* const callbacks, const char* const filename, const ClownCD_FileMode mode)
{
	ClownCD_File file;
	file.functions = callbacks;
	file.stream = file.functions->open(filename, mode);
	return file;
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

int ClownCD_FileSeek(ClownCD_File* const file, const long position, const ClownCD_FileOrigin origin)
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
