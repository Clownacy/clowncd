#include "cue.h"

#include <stdlib.h>

#include "../common.h"
#include "../cue.h"
#include "../utilities.h"

static void ClownCD_SeekTrackIndexCallback(
	void* const user_data,
	const char* const filename,
	const ClownCD_CueFileType file_type,
	const unsigned int track,
	const ClownCD_CueTrackType track_type,
	const unsigned int index,
	const unsigned long sector)
{
	/* TODO: Cache the track filename so that we don't reopen files unnecessarily. */
	ClownCD_Disc* const disc = (ClownCD_Disc*)user_data;
	char* const full_path = ClownCD_GetFullFilePath(disc->filename, filename);

	ClownCD_CloseTrackFile(disc);

	if (full_path != NULL)
	{
		disc->track.file = ClownCD_FileOpen(full_path, CLOWNCD_RB, disc->file.functions);
		free(full_path);

		disc->track.audio_decoder_needed = file_type == CLOWNCD_CUE_FILE_WAVE || file_type == CLOWNCD_CUE_FILE_MP3;
		disc->track.has_full_sized_sectors = track_type != CLOWNCD_CUE_TRACK_MODE1_2048;
		disc->track.starting_frame = sector * CLOWNCD_AUDIO_FRAMES_PER_SECTOR;
		disc->track.total_frames = ClownCD_CueGetTrackIndexEndingSector(&disc->file, filename, track, index, sector) * CLOWNCD_AUDIO_FRAMES_PER_SECTOR - disc->track.starting_frame;

		if (disc->track.audio_decoder_needed)
			if (!ClownCD_AudioOpen(&disc->track.audio, &disc->track.file))
				ClownCD_FileClose(&disc->track.file);
	}
}

cc_bool ClownCD_Disc_CueDetect(ClownCD_File* const file)
{
	return ClownCD_CueIsValid(file);
}

void ClownCD_Disc_CueOpen(ClownCD_Disc* const disc)
{
	disc->track.file = ClownCD_FileOpenBlank();
}

cc_bool ClownCD_Disc_CueSeekTrackIndex(ClownCD_Disc* const disc, const unsigned int track, const unsigned int index)
{
	if (!ClownCD_CueGetTrackIndexInfo(&disc->file, track, index, ClownCD_SeekTrackIndexCallback, disc))
		return cc_false;

	if (!ClownCD_FileIsOpen(&disc->track.file))
		return cc_false;

	return cc_true;
}