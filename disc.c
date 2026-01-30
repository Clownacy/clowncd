#include "disc.h"

#include <assert.h>
#include <string.h>

#include "cue.h"

static ClownCD_DiscType ClownCD_GetDiscType(ClownCD_File* const file)
{
	static const unsigned char header_2352[0x10] = {0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x02, 0x00, 0x01};
	static const unsigned char header_clowncd_v0[0xA] = {0x63, 0x6C, 0x6F, 0x77, 0x6E, 0x63, 0x64, 0x00, 0x00, 0x00};

	unsigned char buffer[0x10];

	const cc_bool read_successful = ClownCD_FileRead(buffer, 0x10, 1, file) == 1;

	if (read_successful && memcmp(buffer, header_2352, sizeof(header_2352)) == 0)
		return CLOWNCD_DISC_RAW_2352;
	else if (read_successful && memcmp(buffer, header_clowncd_v0, sizeof(header_clowncd_v0)) == 0)
		return CLOWNCD_DISC_CLOWNCD;
	else if (ClownCD_CueIsValid(file))
		return CLOWNCD_DISC_CUE;
	else
		return CLOWNCD_DISC_RAW_2048;
}


void ClownCD_CloseTrackFile(ClownCD_Disc* const disc)
{
	if (ClownCD_FileIsOpen(&disc->track.file))
	{
		if (disc->track.audio_decoder_needed)
			ClownCD_AudioClose(&disc->track.audio);

		ClownCD_FileClose(&disc->track.file);
	}
}

void ClownCD_DiscOpen(ClownCD_Disc* const disc)
{
	disc->type = ClownCD_GetDiscType(&disc->file);

	switch (disc->type)
	{
		default:
			assert(cc_false);
			/* Fallthrough */
		case CLOWNCD_DISC_CUE:
			ClownCD_Disc_CueOpen(disc);
			break;

		case CLOWNCD_DISC_RAW_2048:
		case CLOWNCD_DISC_RAW_2352:
			ClownCD_Disc_RawOpen(disc);
			break;

		case CLOWNCD_DISC_CLOWNCD:
			ClownCD_Disc_ClownCDOpen(disc);
			break;
	}
}

cc_bool ClownCD_DiscSeekTrackIndex(ClownCD_Disc* const disc, const unsigned int track, const unsigned int index)
{
	switch (disc->type)
	{
		case CLOWNCD_DISC_CUE:
			return ClownCD_Disc_CueSeekTrackIndex(disc, track, index);

		case CLOWNCD_DISC_RAW_2048:
		case CLOWNCD_DISC_RAW_2352:
			return ClownCD_Disc_RawSeekTrackIndex(disc, track, index, disc->type == CLOWNCD_DISC_RAW_2352);

		case CLOWNCD_DISC_CLOWNCD:
			return ClownCD_Disc_ClownCDSeekTrackIndex(disc, track, index);
	}

	return cc_true;
}