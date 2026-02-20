#ifndef CLOWNCD_WAV_H
#define CLOWNCD_WAV_H

#include <stddef.h>

#define DR_WAV_NO_STDIO
#define DRWAV_PRIVATE static
#include "libraries/dr_wav.h"

#include "../clowncommon/clowncommon.h"

#include "../audio-common.h"
#include "../file-io.h"

typedef struct ClownCD_WAV
{
	drwav dr_wav;
} ClownCD_WAV;

cc_bool ClownCD_WAVOpen(ClownCD_WAV *wav, ClownCD_File *file, ClownCD_AudioMetadata *metadata);
void ClownCD_WAVClose(ClownCD_WAV *wav);

cc_bool ClownCD_WAVSeek(ClownCD_WAV *wav, size_t frame);
size_t ClownCD_WAVRead(ClownCD_WAV *wav, short *buffer, size_t total_frames);

#endif /* CLOWNCD_WAV_H */
