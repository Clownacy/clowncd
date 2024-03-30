#ifndef CUE_H
#define CUE_H

#include "clowncommon/clowncommon.h"

#include "file-io.h"

typedef enum ClownCD_CueFileType
{
	CLOWNCD_CUE_FILE_INVALID,
	CLOWNCD_CUE_FILE_BINARY
} ClownCD_CueFileType;

typedef enum ClownCD_CueTrackType
{
	CLOWNCD_CUE_TRACK_INVALID,
	CLOWNCD_CUE_TRACK_MODE1_2048,
	CLOWNCD_CUE_TRACK_MODE1_2352,
	CLOWNCD_CUE_TRACK_AUDIO
} ClownCD_CueTrackType;

typedef void (*ClownCD_CueCallback)(void *user_data, const char *filename, ClownCD_CueFileType file_type, unsigned int track, ClownCD_CueTrackType track_type, unsigned int index, unsigned long frame);

void ClownCD_CueParse(ClownCD_File *file, ClownCD_CueCallback callback, const void *user_data);
cc_bool ClownCD_CueGetTrackIndexInfo(ClownCD_File *file, unsigned int track, unsigned int index, ClownCD_CueCallback callback, const void *user_data);

#endif /* CUE_H */
