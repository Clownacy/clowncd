#ifndef CLOWNCD_MP3_H
#define CLOWNCD_MP3_H

#include <stddef.h>

#define DR_MP3_NO_STDIO
#define DRMP3_API static
#define DRMP3_PRIVATE static
#include "libraries/dr_mp3.h"

#include "../clowncommon/clowncommon.h"

#include "../file-io.h"

typedef struct ClownCD_MP3
{
	drmp3 dr_mp3;
} ClownCD_MP3;

cc_bool ClownCD_MP3Open(ClownCD_MP3 *mp3, ClownCD_File *file);
void ClownCD_MP3Close(ClownCD_MP3 *mp3);

cc_bool ClownCD_MP3Seek(ClownCD_MP3 *mp3, size_t frame);
size_t ClownCD_MP3Read(ClownCD_MP3 *mp3, short *buffer, size_t total_frames);

#endif /* CLOWNCD_MP3_H */
