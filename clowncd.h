#ifndef CLOWNCD_H
#define CLOWNCD_H

#include <stddef.h>

#include "clowncommon/clowncommon.h"

#include "audio.h"
#include "common.h"
#include "cue.h"
#include "disc.h"
#include "error.h"
#include "file-io.h"

typedef struct ClownCD
{
	ClownCD_Disc disc;
} ClownCD;

#ifdef __cplusplus
extern "C" {
#endif

#define ClownCD_Open(state, file_path, callbacks) ClownCD_OpenAlreadyOpen(state, NULL, file_path, callbacks)
void ClownCD_OpenAlreadyOpen(ClownCD *state, void *stream, const char *file_path, const ClownCD_FileCallbacks *callbacks);
void ClownCD_Close(ClownCD *state);
#define ClownCD_IsOpen(state) ClownCD_FileIsOpen(&(state)->file)

#define ClownCD_SeekTrackIndex(state, track, index) ClownCD_SetState(state, track, index, 0)
#define ClownCD_SeekSector(state, sector) ClownCD_SeekAudioFrame((state), (size_t)(sector) * CLOWNCD_AUDIO_FRAMES_PER_SECTOR)
cc_bool ClownCD_SeekAudioFrame(ClownCD *state, size_t frame);

cc_bool ClownCD_SetState(ClownCD *state, unsigned int track, unsigned int index, size_t frame);

cc_bool ClownCD_BeginSectorStream(ClownCD* state);
size_t ClownCD_ReadSectorStream(ClownCD* state, unsigned char *buffer, size_t total_bytes);
cc_bool ClownCD_EndSectorStream(ClownCD* state);
size_t ClownCD_ReadSector(ClownCD* state, unsigned char *buffer);

size_t ClownCD_ReadFrames(ClownCD *state, short *buffer, size_t total_frames);

unsigned long ClownCD_CalculateSectorCRC(const unsigned char *buffer);
cc_bool ClownCD_ValidateSectorCRC(const unsigned char *buffer);

#ifdef __cplusplus
}
#endif

#endif /* CLOWNCD_H */
