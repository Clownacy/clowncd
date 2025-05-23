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
		ClownCD_CueFileType file_type;
		ClownCD_CueTrackType type;
		size_t starting_frame, current_frame, total_frames;
		unsigned long starting_sector, ending_sector, current_sector;
		unsigned int current_track, current_index;
		ClownCD_Audio audio;
	} track;
} ClownCD;

#ifdef __cplusplus
extern "C" {
#endif

ClownCD ClownCD_Open(const char *file_path, const ClownCD_FileCallbacks *callbacks);
ClownCD ClownCD_OpenAlreadyOpen(void *stream, const char *file_path, const ClownCD_FileCallbacks *callbacks);
void ClownCD_Close(ClownCD *disc);
#define ClownCD_IsOpen(disc) ClownCD_FileIsOpen(&(disc)->file)

ClownCD_CueTrackType ClownCD_SeekTrackIndex(ClownCD *disc, unsigned int track, unsigned int index);
cc_bool ClownCD_SeekSector(ClownCD *disc, unsigned long sector);
cc_bool ClownCD_SeekAudioFrame(ClownCD *disc, size_t frame);

ClownCD_CueTrackType ClownCD_SetState(ClownCD *disc, unsigned int track, unsigned int index, unsigned long sector, size_t frame);

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
