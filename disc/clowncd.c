#include "clowncd.h"

#include <assert.h>

#include "../common.h"

static size_t ClownCD_ClownCDTrackMetadataOffset(const unsigned int track)
{
	assert(track != 0);
	return 12 + (track - 1) * 10;
}

static size_t ClownCD_GetHeaderSize(ClownCD_Disc* const disc)
{
	if (ClownCD_FileSeek(&disc->track.file, 10, CLOWNCD_SEEK_SET) != 0)
		return -1;

	return ClownCD_ClownCDTrackMetadataOffset(ClownCD_ReadU16BE(&disc->track.file) + 1);
}

void ClownCD_Disc_ClownCDOpen(ClownCD_Disc* const disc)
{
	disc->track.file = disc->file;
	disc->file = ClownCD_FileOpenBlank();

	disc->track.header_size = ClownCD_GetHeaderSize(disc);
}

cc_bool ClownCD_Disc_ClownCDSeekTrackIndex(ClownCD_Disc* const disc, const unsigned int track, const unsigned int index)
{
	if (index != 1)
		return cc_false;

	if (ClownCD_FileSeek(&disc->track.file, 10, CLOWNCD_SEEK_SET) != 0)
		return cc_false;

	if (track >= ClownCD_ReadU32BE(&disc->track.file))
		return cc_false;

	if (ClownCD_FileSeek(&disc->track.file, ClownCD_ClownCDTrackMetadataOffset(track), CLOWNCD_SEEK_SET) != 0)
		return cc_false;

	disc->track.audio_decoder_needed = cc_false;
	disc->track.has_full_sized_sectors = cc_true;
	ClownCD_ReadU16BE(&disc->track.file); /* The unused track type value. */
	disc->track.starting_frame = ClownCD_ReadU32BE(&disc->track.file) * CLOWNCD_AUDIO_FRAMES_PER_SECTOR;
	disc->track.total_frames = ClownCD_ReadU32BE(&disc->track.file) * CLOWNCD_AUDIO_FRAMES_PER_SECTOR;

	return cc_true;
}