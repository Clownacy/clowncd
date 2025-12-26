#include "file-io.h"

#include <assert.h>

#include <stdio.h>

static void* ClownCD_FileOpenStandard(const char* const filename, const ClownCD_FileMode mode)
{
	const char *standard_mode;

	switch (mode)
	{
		case CLOWNCD_RB:
			standard_mode = "rb";
			break;

		case CLOWNCD_WB:
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

ClownCD_File ClownCD_FileOpenBlank(void)
{
	ClownCD_File file;
	file.functions = NULL;
	file.stream = NULL;
	file.eof = cc_false;
	file.already_open = cc_false;
	return file;
}

static const ClownCD_FileCallbacks* ClownCD_GetCallbacks(const ClownCD_FileCallbacks *callbacks)
{
	static const ClownCD_FileCallbacks standard_callbacks = {
		ClownCD_FileOpenStandard,
		ClownCD_FileCloseStandard,
		ClownCD_FileReadStandard,
		ClownCD_FileWriteStandard,
		ClownCD_FileTellStandard,
		ClownCD_FileSeekStandard
	};

	return callbacks != NULL ? callbacks : &standard_callbacks;
}

ClownCD_File ClownCD_FileOpen(const char* const filename, const ClownCD_FileMode mode, const ClownCD_FileCallbacks* const callbacks)
{
	ClownCD_File file = ClownCD_FileOpenBlank();
	file.functions = ClownCD_GetCallbacks(callbacks);
	file.stream = ClownCD_GetCallbacks(callbacks)->open(filename, mode);
	return file;
}

ClownCD_File ClownCD_FileOpenAlreadyOpen(void* const stream, const ClownCD_FileCallbacks* const callbacks)
{
	ClownCD_File file = ClownCD_FileOpenBlank();
	file.functions = ClownCD_GetCallbacks(callbacks);
	file.stream = stream;
	file.already_open = cc_true;
	return file;
}

int ClownCD_FileClose(ClownCD_File* const file)
{
	if (ClownCD_FileIsOpen(file))
	{
		void* const stream = file->stream;
		file->stream = NULL;

		if (!file->already_open)
			return file->functions->close(stream);
	}

	return 0;
}

size_t ClownCD_FileRead(void* const buffer, const size_t size, const size_t count, ClownCD_File* const file)
{
	if (!ClownCD_FileIsOpen(file))
	{
		file->eof = cc_true;
		return 0;
	}
	else
	{
		const size_t total_done = file->functions->read(buffer, size, count, file->stream);

		if (total_done != count)
			file->eof = cc_true;

		return total_done;
	}
}

size_t ClownCD_FileWrite(const void* const buffer, const size_t size, const size_t count, ClownCD_File* const file)
{
	if (!ClownCD_FileIsOpen(file))
		return 0;
	else
		return file->functions->write(buffer, size, count, file->stream);
}

long ClownCD_FileTell(ClownCD_File* const file)
{
	if (!ClownCD_FileIsOpen(file))
		return -1L;
	else
		return file->functions->tell(file->stream);
}

int ClownCD_FileSeek(ClownCD_File* const file, const long position, const ClownCD_FileOrigin origin)
{
	if (!ClownCD_FileIsOpen(file))
	{
		return -1;
	}
	else
	{
		file->eof = cc_false;
		return file->functions->seek(file->stream, position, origin);
	}
}

size_t ClownCD_FileSize(ClownCD_File* const file)
{
	size_t file_size = CLOWNCD_SIZE_INVALID;

	if (ClownCD_FileIsOpen(file))
	{
		const long position = ClownCD_FileTell(file);

		if (position != -1L)
		{
			if (ClownCD_FileSeek(file, 0, CLOWNCD_SEEK_END) == 0)
				file_size = ClownCD_FileTell(file);

			ClownCD_FileSeek(file, position, CLOWNCD_SEEK_SET);
		}
	}

	return file_size;
}

void ClownCD_WriteUintMemory(unsigned char* const buffer, const unsigned long value, const unsigned int total_bytes, const cc_bool big_endian)
{
	unsigned long value_shifter = value;
	unsigned int i;

	for (i = 0; i < total_bytes; ++i)
	{
		buffer[big_endian ? total_bytes - i - 1 : i] = value_shifter & 0xFF;
		value_shifter >>= 8;
	}
}

unsigned long ClownCD_WriteUintFile(ClownCD_File* const file, const unsigned long value, const unsigned int total_bytes, const cc_bool big_endian)
{
	unsigned char buffer[4];

	if (total_bytes > CC_COUNT_OF(buffer))
		return 0;

	if (total_bytes > sizeof(unsigned long) || (total_bytes < sizeof(unsigned long) && value > (1UL << total_bytes * 8) - 1))
		return 0;

	ClownCD_WriteUintMemory(buffer, value, total_bytes, big_endian);
	if (ClownCD_FileWrite(buffer, total_bytes, 1, file) != 1)
		return 0;

	return value;
}

static unsigned long ClownCD_SignedLongToUnsignedLong(const signed long value, const unsigned int total_bytes)
{
	if (value < 0)
	{
		const unsigned long sign_bit_mask = 1UL << (total_bytes * 8 - 1);
		const unsigned long size_mask = sign_bit_mask | (sign_bit_mask - 1);

		const unsigned long absolute_value = -value;

		return (0UL - absolute_value) & size_mask;
	}
	else
	{
		return value;
	}
}

void ClownCD_WriteSintMemory(unsigned char* const buffer, const unsigned long value, const unsigned int total_bytes, const cc_bool big_endian)
{
	ClownCD_WriteUintMemory(buffer, ClownCD_SignedLongToUnsignedLong(value, total_bytes), total_bytes, big_endian);
}

void ClownCD_WriteSintFile(ClownCD_File* const file, const unsigned long value, const unsigned int total_bytes, const cc_bool big_endian)
{
	ClownCD_WriteUintFile(file, ClownCD_SignedLongToUnsignedLong(value, total_bytes), total_bytes, big_endian);
}

unsigned long ClownCD_ReadUintMemory(const unsigned char* const buffer, const unsigned int total_bytes, const cc_bool big_endian)
{
	unsigned long value = 0;
	unsigned int i;

	for (i = 0; i < total_bytes; ++i)
	{
		value <<= 8;
		value |= buffer[big_endian ? i : total_bytes - i - 1];
	}

	return value;
}

unsigned long ClownCD_ReadUintFile(ClownCD_File* const file, const unsigned int total_bytes, const cc_bool big_endian)
{
	unsigned char buffer[4];

	if (total_bytes > CC_COUNT_OF(buffer))
		return 0;

	if (ClownCD_FileRead(buffer, total_bytes, 1, file) != 1)
		return 0;

	return ClownCD_ReadUintMemory(buffer, total_bytes, big_endian);
}

static signed long ClownCD_UnsignedLongToSignedLong(const unsigned long value, const unsigned int total_bytes)
{
	const unsigned long sign_bit_mask = 1UL << (total_bytes * 8 - 1);

	if ((value & sign_bit_mask) != 0)
	{
		return -(signed long)(sign_bit_mask - (value & (sign_bit_mask - 1)));
	}
	else
	{
		return value;
	}
}

signed long ClownCD_ReadSintMemory(const unsigned char* const buffer, const unsigned int total_bytes, const cc_bool big_endian)
{
	return ClownCD_UnsignedLongToSignedLong(ClownCD_ReadUintMemory(buffer, total_bytes, big_endian), total_bytes);
}

signed long ClownCD_ReadSintFile(ClownCD_File* const file, const unsigned int total_bytes, const cc_bool big_endian)
{
	return ClownCD_UnsignedLongToSignedLong(ClownCD_ReadUintFile(file, total_bytes, big_endian), total_bytes);
}
