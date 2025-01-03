#ifndef CLOWNCD_FLAC_H
#define CLOWNCD_FLAC_H

#include <stddef.h>

#define DR_FLAC_NO_STDIO
#define DRFLAC_API static
#define DRFLAC_PRIVATE static
#include "libraries/dr_flac.h"

#include "../clowncommon/clowncommon.h"

#include "../audio-common.h"
#include "../file-io.h"

typedef struct ClownCD_FLAC
{
	drflac *dr_flac;
} ClownCD_FLAC;

cc_bool ClownCD_FLACOpen(ClownCD_FLAC *flac, ClownCD_File *file, ClownCD_AudioMetadata *metadata);
void ClownCD_FLACClose(ClownCD_FLAC *flac);

cc_bool ClownCD_FLACSeek(ClownCD_FLAC *flac, size_t frame);
size_t ClownCD_FLACRead(ClownCD_FLAC *flac, short *buffer, size_t total_frames);

#endif /* CLOWNCD_FLAC_H */
