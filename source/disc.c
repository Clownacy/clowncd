#include "disc.h"

#include <assert.h>

#include "disc/clowncd.h"
#ifdef CLOWNCD_CHD
#include "disc/chd.h"
#endif
#include "disc/cue.h"
#include "disc/raw.h"

static ClownCD_DiscType ClownCD_GetDiscType(ClownCD_File* const file)
{
#ifdef CLOWNCD_CHD
	if (ClownCD_Disc_CHDDetect(file))
		return CLOWNCD_DISC_CHD;
	else
#endif
	if (ClownCD_Disc_RawDetect(file))
		return CLOWNCD_DISC_RAW_2352;
	else if (ClownCD_Disc_ClownCDDetect(file))
		return CLOWNCD_DISC_CLOWNCD;
	else if (ClownCD_Disc_CueDetect(file))
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
#ifdef CLOWNCD_CHD
		case CLOWNCD_DISC_CHD:
			ClownCD_Disc_CHDOpen(disc);
			break;
#endif
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
		case CLOWNCD_DISC_CHD:
#ifdef CLOWNCD_CHD
			return ClownCD_Disc_CHDSeekTrackIndex(disc, track, index);
#else
			break;
#endif
	}

	return cc_true;
}
