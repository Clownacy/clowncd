#ifndef FILE_IO_H
#define FILE_IO_H

#include <stddef.h>

#include "clowncommon/clowncommon.h"

#define CLOWNCD_EOF -1

enum
{
	CLOWNCD_MODE_READ,
	CLOWNCD_MODE_WRITE
};

enum
{
	CLOWNCD_SEEK_SET,
	CLOWNCD_SEEK_CUR,
	CLOWNCD_SEEK_END
};

typedef struct ClownCD_FileCallbacks
{
	void* (*open)(const char *filename, int mode);
	int (*close)(void *stream);
	size_t (*read)(void *buffer, size_t size, size_t count, void *stream);
	size_t (*write)(const void *buffer, size_t size, size_t count, void *stream);
	long (*tell)(void *stream);
	int (*seek)(void *stream, long position, int origin);
} ClownCD_FileCallbacks;

typedef struct ClownCD_File
{
	const ClownCD_FileCallbacks *functions;
	void *stream;
} ClownCD_File;

void ClownCD_FileOpen(ClownCD_File *file, const char *filename, int mode);
int ClownCD_FileClose(ClownCD_File *file);
size_t ClownCD_FileRead(void *buffer, size_t size, size_t count, ClownCD_File *file);
size_t ClownCD_FileWrite(const void *buffer, size_t size, size_t count, ClownCD_File *file);
long ClownCD_FileTell(ClownCD_File* const file);
int ClownCD_FileSeek(ClownCD_File* const file, long position, int origin);

void WriteMemory(unsigned char *buffer, unsigned long value, unsigned int total_bytes, cc_bool big_endian);
void WriteFile(ClownCD_File *file, unsigned long value, unsigned int total_bytes, cc_bool big_endian);
unsigned long ReadMemory(const unsigned char *buffer, unsigned int total_bytes, cc_bool big_endian);
unsigned long ReadFile(ClownCD_File *file, unsigned int total_bytes, cc_bool big_endian);

#define Write16LEMemory(buffer, value) WriteMemory(buffer, value, 2, cc_false)
#define Write32LEMemory(buffer, value) WriteMemory(buffer, value, 4, cc_false)

#define Write16LE(file, value) WriteFile(file, value, 2, cc_false)
#define Write32LE(file, value) WriteFile(file, value, 4, cc_false)

#define Write16BEMemory(buffer, value) WriteMemory(buffer, value, 2, cc_true)
#define Write32BEMemory(buffer, value) WriteMemory(buffer, value, 4, cc_true)

#define Write16BE(file, value) WriteFile(file, value, 2, cc_true)
#define Write32BE(file, value) WriteFile(file, value, 4, cc_true)

#define Write8(file, value) WriteFile(file, value, 1, cc_true)

#define Read16LEMemory(buffer) ReadMemory(buffer, 2, cc_false)
#define Read32LEMemory(buffer) ReadMemory(buffer, 4, cc_false)

#define Read16LE(file) ReadFile(file, 2, cc_false)
#define Read32LE(file) ReadFile(file, 4, cc_false)

#define Read16BEMemory(buffer) ReadMemory(buffer, 2, cc_true)
#define Read32BEMemory(buffer) ReadMemory(buffer, 4, cc_true)

#define Read16BE(file) ReadFile(file, 2, cc_true)
#define Read32BE(file) ReadFile(file, 4, cc_true)

#define Read8(file) ReadFile(file, 1, cc_true)

#endif /* FILE_IO_H */
