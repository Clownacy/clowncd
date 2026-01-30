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

typedef ClownCD_Disc ClownCD;

#ifdef __cplusplus
extern "C" {
#endif

#define ClownCD_Open(disc, file_path, callbacks) ClownCD_OpenAlreadyOpen(disc, NULL, file_path, callbacks)
void ClownCD_OpenAlreadyOpen(ClownCD *disc, void *stream, const char *file_path, const ClownCD_FileCallbacks *callbacks);
void ClownCD_Close(ClownCD *disc);
#define ClownCD_IsOpen(disc) ClownCD_FileIsOpen(&(disc)->file)

#define ClownCD_SeekTrackIndex(disc, track, index) ClownCD_SetState(disc, track, index, 0)
#define ClownCD_SeekSector(disc, sector) ClownCD_SeekAudioFrame((disc), (size_t)(sector) * CLOWNCD_AUDIO_FRAMES_PER_SECTOR)
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
