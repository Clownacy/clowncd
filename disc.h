#ifndef CLOWNCD_DISC_H
#define CLOWNCD_DISC_H

#include "audio.h"
#include "file-io.h"

typedef enum ClownCD_DiscType
{
	CLOWNCD_DISC_CUE,
	CLOWNCD_DISC_RAW_2048,
	CLOWNCD_DISC_RAW_2352,
	CLOWNCD_DISC_CLOWNCD
} ClownCD_DiscType;

typedef struct ClownCD_Disc
{
	ClownCD_DiscType type;
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

void ClownCD_DiscOpen(ClownCD_Disc *disc);
cc_bool ClownCD_DiscSeekTrackIndex(ClownCD_Disc *disc, unsigned int track, unsigned int index);

#endif /* CLOWNCD_DISC_H */
