#ifndef CLOWNCD_H
#define CLOWNCD_H

#include <stddef.h>

#include "clowncommon/clowncommon.h"

#include "audio.h"
#include "cue.h"
#include "error.h"
#include "file-io.h"

#define CLOWNCD_SECTOR_RAW_SIZE 2352
#define CLOWNCD_SECTOR_HEADER_SIZE 0x10
#define CLOWNCD_SECTOR_DATA_SIZE 0x800
#define CLOWNCD_AUDIO_CHANNELS 2
#define CLOWNCD_AUDIO_FRAME_SIZE (CLOWNCD_AUDIO_CHANNELS * 2)
#define CLOWNCD_AUDIO_FRAMES_PER_SECTOR (CLOWNCD_SECTOR_RAW_SIZE / CLOWNCD_AUDIO_FRAME_SIZE)

typedef enum ClownCD_DiscType
{
	CLOWNCD_DISC_CUE,
	CLOWNCD_DISC_RAW_2048,
	CLOWNCD_DISC_RAW_2352,
	CLOWNCD_DISC_CLOWNCD
} ClownCD_DiscType;

typedef struct ClownCD
{
	char *filename;
	ClownCD_File file;
	ClownCD_DiscType type;
	struct
	{
		ClownCD_File file;
		cc_bool audio_decoder_needed, has_full_sized_sectors;
		size_t starting_frame, current_frame, total_frames;
		unsigned int current_track, current_index;
		ClownCD_Audio audio;
	} track;
} ClownCD;

#ifdef __cplusplus
extern "C" {
#endif

#define ClownCD_Open(file_path, callbacks) ClownCD_OpenAlreadyOpen(NULL, file_path, callbacks)
ClownCD ClownCD_OpenAlreadyOpen(void *stream, const char *file_path, const ClownCD_FileCallbacks *callbacks);
void ClownCD_Close(ClownCD *disc);
#define ClownCD_IsOpen(disc) ClownCD_FileIsOpen(&(disc)->file)

#define ClownCD_SeekTrackIndex(disc, track, index) ClownCD_SetState(disc, track, index, 0)
#define ClownCD_SeekSector(disc, sector) ClownCD_SeekAudioFrame((disc), (sector) * CLOWNCD_AUDIO_FRAMES_PER_SECTOR)
cc_bool ClownCD_SeekAudioFrame(ClownCD *disc, size_t frame);

cc_bool ClownCD_SetState(ClownCD *disc, unsigned int track, unsigned int index, size_t frame);

cc_bool ClownCD_BeginSectorStream(ClownCD* disc);
size_t ClownCD_ReadSectorStream(ClownCD* disc, unsigned char *buffer, size_t total_bytes);
cc_bool ClownCD_EndSectorStream(ClownCD* disc);
size_t ClownCD_ReadSector(ClownCD* disc, unsigned char *buffer);

size_t ClownCD_ReadFrames(ClownCD *disc, short *buffer, size_t total_frames);

unsigned long ClownCD_CalculateSectorCRC(const unsigned char *buffer);
cc_bool ClownCD_ValidateSectorCRC(const unsigned char *buffer);

#ifdef __cplusplus
}
#endif

#endif /* CLOWNCD_H */
