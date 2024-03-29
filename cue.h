#ifndef CUE_H
#define CUE_H

#include <stdio.h>

#include "clowncommon/clowncommon.h"

typedef enum Cue_FileType
{
	CUE_FILE_TYPE_INVALID,
	CUE_FILE_TYPE_BINARY
} Cue_FileType;

typedef enum Cue_TrackType
{
	CUE_TRACK_TYPE_INVALID,
	CUE_TRACK_TYPE_MODE1_2048,
	CUE_TRACK_TYPE_MODE1_2352,
	CUE_TRACK_TYPE_AUDIO
} Cue_TrackType;

typedef void (*Cue_Callback)(void *user_data, const char *filename, Cue_FileType file_type, unsigned int track, Cue_TrackType track_type, unsigned int index, unsigned long frame);

void Cue_Parse(FILE *file, Cue_Callback callback, const void *user_data);
cc_bool Cue_GetTrackIndexInfo(FILE *file, unsigned int track, unsigned int index, Cue_Callback callback, const void *user_data);

#endif /* CUE_H */
