#ifndef FILE_IO_H
#define FILE_IO_H

#include <stddef.h>

#include "clowncommon/clowncommon.h"

#define CLOWNCD_EOF -1

typedef enum ClownCD_FileMode
{
	CLOWNCD_MODE_READ,
	CLOWNCD_MODE_WRITE
} ClownCD_FileMode;

typedef enum ClownCD_FileOrigin
{
	CLOWNCD_SEEK_SET,
	CLOWNCD_SEEK_CUR,
	CLOWNCD_SEEK_END
} ClownCD_FileOrigin;

typedef struct ClownCD_FileCallbacks
{
	void* (*open)(const char *filename, ClownCD_FileMode mode);
	int (*close)(void *stream);
	size_t (*read)(void *buffer, size_t size, size_t count, void *stream);
	size_t (*write)(const void *buffer, size_t size, size_t count, void *stream);
	long (*tell)(void *stream);
	int (*seek)(void *stream, long position, ClownCD_FileOrigin origin);
} ClownCD_FileCallbacks;

typedef struct ClownCD_File
{
	const ClownCD_FileCallbacks *functions;
	void *stream;
} ClownCD_File;

ClownCD_File ClownCD_FileOpen(const char* const filename, const ClownCD_FileMode mode);
ClownCD_File ClownCD_FileOpenCustomIO(const ClownCD_FileCallbacks *callbacks, const char *filename, ClownCD_FileMode mode);
int ClownCD_FileClose(ClownCD_File *file);
size_t ClownCD_FileRead(void *buffer, size_t size, size_t count, ClownCD_File *file);
size_t ClownCD_FileWrite(const void *buffer, size_t size, size_t count, ClownCD_File *file);
long ClownCD_FileTell(ClownCD_File* const file);
int ClownCD_FileSeek(ClownCD_File* const file, long position, ClownCD_FileOrigin origin);

void ClownCD_WriteMemory(unsigned char *buffer, unsigned long value, unsigned int total_bytes, cc_bool big_endian);
void ClownCD_WriteFile(ClownCD_File *file, unsigned long value, unsigned int total_bytes, cc_bool big_endian);
unsigned long ClownCD_ReadMemory(const unsigned char *buffer, unsigned int total_bytes, cc_bool big_endian);
unsigned long ClownCD_ReadFile(ClownCD_File *file, unsigned int total_bytes, cc_bool big_endian);

#define ClownCD_Write16LEMemory(buffer, value) ClownCD_WriteMemory(buffer, value, 2, cc_false)
#define ClownCD_Write32LEMemory(buffer, value) ClownCD_WriteMemory(buffer, value, 4, cc_false)

#define ClownCD_Write16LE(file, value) ClownCD_WriteFile(file, value, 2, cc_false)
#define ClownCD_Write32LE(file, value) ClownCD_WriteFile(file, value, 4, cc_false)

#define ClownCD_Write16BEMemory(buffer, value) ClownCD_WriteMemory(buffer, value, 2, cc_true)
#define ClownCD_Write32BEMemory(buffer, value) ClownCD_WriteMemory(buffer, value, 4, cc_true)

#define ClownCD_Write16BE(file, value) ClownCD_WriteFile(file, value, 2, cc_true)
#define ClownCD_Write32BE(file, value) ClownCD_WriteFile(file, value, 4, cc_true)

#define ClownCD_Write8(file, value) ClownCD_WriteFile(file, value, 1, cc_true)

#define ClownCD_Read16LEMemory(buffer) ClownCD_ReadMemory(buffer, 2, cc_false)
#define ClownCD_Read32LEMemory(buffer) ClownCD_ReadMemory(buffer, 4, cc_false)

#define ClownCD_Read16LE(file) ClownCD_ReadFile(file, 2, cc_false)
#define ClownCD_Read32LE(file) ClownCD_ReadFile(file, 4, cc_false)

#define ClownCD_Read16BEMemory(buffer) ClownCD_ReadMemory(buffer, 2, cc_true)
#define ClownCD_Read32BEMemory(buffer) ClownCD_ReadMemory(buffer, 4, cc_true)

#define ClownCD_Read16BE(file) ClownCD_ReadFile(file, 2, cc_true)
#define ClownCD_Read32BE(file) ClownCD_ReadFile(file, 4, cc_true)

#define ClownCD_Read8(file) ClownCD_ReadFile(file, 1, cc_true)

#endif /* FILE_IO_H */
