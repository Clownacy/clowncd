#ifndef CLOWNCD_FILE_IO_H
#define CLOWNCD_FILE_IO_H

#include <stddef.h>

#include "clowncommon/clowncommon.h"

#define CLOWNCD_EOF -1

typedef enum ClownCD_FileMode
{
	CLOWNCD_RB,
	CLOWNCD_WB
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
size_t ClownCD_FileSize(ClownCD_File *file);
#define ClownCD_FileIsOpen(file) ((file)->stream != NULL)

void ClownCD_WriteMemory(unsigned char *buffer, unsigned long value, unsigned int total_bytes, cc_bool big_endian);
unsigned long ClownCD_WriteFile(ClownCD_File *file, unsigned long value, unsigned int total_bytes, cc_bool big_endian);
unsigned long ClownCD_ReadUintMemory(const unsigned char *buffer, unsigned int total_bytes, cc_bool big_endian);
unsigned long ClownCD_ReadUintFile(ClownCD_File *file, unsigned int total_bytes, cc_bool big_endian);
signed long ClownCD_ReadSintMemory(const unsigned char *buffer, unsigned int total_bytes, cc_bool big_endian);
signed long ClownCD_ReadSintFile(ClownCD_File *file, unsigned int total_bytes, cc_bool big_endian);

#define ClownCD_WriteU16LEMemory(buffer, value) ClownCD_WriteMemory(buffer, value, 2, cc_false)
#define ClownCD_WriteU32LEMemory(buffer, value) ClownCD_WriteMemory(buffer, value, 4, cc_false)

#define ClownCD_WriteU16LE(file, value) ClownCD_WriteFile(file, value, 2, cc_false)
#define ClownCD_WriteU32LE(file, value) ClownCD_WriteFile(file, value, 4, cc_false)

#define ClownCD_WriteU16BEMemory(buffer, value) ClownCD_WriteMemory(buffer, value, 2, cc_true)
#define ClownCD_WriteU32BEMemory(buffer, value) ClownCD_WriteMemory(buffer, value, 4, cc_true)

#define ClownCD_WriteU16BE(file, value) ClownCD_WriteFile(file, value, 2, cc_true)
#define ClownCD_WriteU32BE(file, value) ClownCD_WriteFile(file, value, 4, cc_true)

#define ClownCD_WriteU8(file, value) ClownCD_WriteFile(file, value, 1, cc_true)

#define ClownCD_ReadU16LEMemory(buffer) ClownCD_ReadUintMemory(buffer, 2, cc_false)
#define ClownCD_ReadU32LEMemory(buffer) ClownCD_ReadUintMemory(buffer, 4, cc_false)

#define ClownCD_ReadU16LE(file) ClownCD_ReadUintFile(file, 2, cc_false)
#define ClownCD_ReadU32LE(file) ClownCD_ReadUintFile(file, 4, cc_false)

#define ClownCD_ReadU16BEMemory(buffer) ClownCD_ReadUintMemory(buffer, 2, cc_true)
#define ClownCD_ReadU32BEMemory(buffer) ClownCD_ReadUintMemory(buffer, 4, cc_true)

#define ClownCD_ReadU16BE(file) ClownCD_ReadUintFile(file, 2, cc_true)
#define ClownCD_ReadU32BE(file) ClownCD_ReadUintFile(file, 4, cc_true)

#define ClownCD_ReadU8(file) ClownCD_ReadUintFile(file, 1, cc_true)

#define ClownCD_ReadS16LEMemory(buffer) ClownCD_ReadSintMemory(buffer, 2, cc_false)
#define ClownCD_ReadS32LEMemory(buffer) ClownCD_ReadSintMemory(buffer, 4, cc_false)

#define ClownCD_ReadS16LE(file) ClownCD_ReadSintFile(file, 2, cc_false)
#define ClownCD_ReadS32LE(file) ClownCD_ReadSintFile(file, 4, cc_false)

#define ClownCD_ReadS16BEMemory(buffer) ClownCD_ReadSintMemory(buffer, 2, cc_true)
#define ClownCD_ReadS32BEMemory(buffer) ClownCD_ReadSintMemory(buffer, 4, cc_true)

#define ClownCD_ReadS16BE(file) ClownCD_ReadSintFile(file, 2, cc_true)
#define ClownCD_ReadS32BE(file) ClownCD_ReadSintFile(file, 4, cc_true)

#define ClownCD_ReadS8(file) ClownCD_ReadSintFile(file, 1, cc_true)

char* ClownCD_GetFullFilePath(const char *directory, const char *filename);

#endif /* CLOWNCD_FILE_IO_H */
