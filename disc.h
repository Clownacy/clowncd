#ifndef CLOWNCD_DISC_H
#define CLOWNCD_DISC_H

#include "audio.h"
#include "file-io.h"

typedef struct ClownCD_Disc
{
	char *filename;
	ClownCD_File file;
	struct
	{
		ClownCD_File file;
		unsigned char header_size;
		cc_bool audio_decoder_needed, has_full_sized_sectors;
		size_t starting_frame, current_frame, total_frames;
		unsigned int current_track, current_index;
		ClownCD_Audio audio;
	} track;
} ClownCD_Disc;

void ClownCD_CloseTrackFile(ClownCD_Disc *disc);

#endif /* CLOWNCD_DISC_H */
