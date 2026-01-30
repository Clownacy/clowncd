#include "disc.h"

void ClownCD_CloseTrackFile(ClownCD_Disc* const disc)
{
	if (ClownCD_FileIsOpen(&disc->track.file))
	{
		if (disc->track.audio_decoder_needed)
			ClownCD_AudioClose(&disc->track.audio);

		ClownCD_FileClose(&disc->track.file);
	}
}
