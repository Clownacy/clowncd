#ifndef FILE_IO_H
#define FILE_IO_H

#include <stdio.h>

#include "clowncommon/clowncommon.h"

void WriteMemory(unsigned char *buffer, unsigned long value, unsigned int total_bytes, cc_bool big_endian);
void WriteFile(FILE *file, unsigned long value, unsigned int total_bytes, cc_bool big_endian);
unsigned long ReadMemory(const unsigned char *buffer, unsigned int total_bytes, cc_bool big_endian);
unsigned long ReadFile(FILE *file, unsigned int total_bytes, cc_bool big_endian);

#define Write16LEMemory(buffer, value) WriteMemory(buffer, value, 2, cc_false)
#define Write32LEMemory(buffer, value) WriteMemory(buffer, value, 4, cc_false)

#define Write16LE(file, value) WriteFile(file, value, 2, cc_false)
#define Write32LE(file, value) WriteFile(file, value, 4, cc_false)

#define Write16BEMemory(buffer, value) WriteMemory(buffer, value, 2, cc_true)
#define Write32BEMemory(buffer, value) WriteMemory(buffer, value, 4, cc_true)

#define Write16BE(file, value) WriteFile(file, value, 2, cc_true)
#define Write32BE(file, value) WriteFile(file, value, 4, cc_true)

#define Read16LEMemory(buffer) ReadMemory(buffer, 2, cc_false)
#define Read32LEMemory(buffer) ReadMemory(buffer, 4, cc_false)

#define Read16LE(file) ReadFile(file, 2, cc_false)
#define Read32LE(file) ReadFile(file, 4, cc_false)

#define Read16BEMemory(buffer) ReadMemory(buffer, 2, cc_true)
#define Read32BEMemory(buffer) ReadMemory(buffer, 4, cc_true)

#define Read16BE(file) ReadFile(file, 2, cc_true)
#define Read32BE(file) ReadFile(file, 4, cc_true)

#endif /* FILE_IO_H */
