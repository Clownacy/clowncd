#ifndef CLOWNCD_VORBIS_H
#define CLOWNCD_VORBIS_H

#include <stddef.h>

#include "../clowncommon/clowncommon.h"

#include "../audio-common.h"
#include "../file-io.h"

struct clowncd_stb_vorbis;

typedef struct ClownCD_Vorbis
{
	void *file_buffer;
	struct clowncd_stb_vorbis *instance;
} ClownCD_Vorbis;

cc_bool ClownCD_VorbisOpen(ClownCD_Vorbis *vorbis, ClownCD_File *file, ClownCD_AudioMetadata *metadata);
void ClownCD_VorbisClose(ClownCD_Vorbis *vorbis);

cc_bool ClownCD_VorbisSeek(ClownCD_Vorbis *vorbis, size_t frame);
size_t ClownCD_VorbisRead(ClownCD_Vorbis *vorbis, short *buffer, size_t total_frames);

#endif /* CLOWNCD_VORBIS_H */
