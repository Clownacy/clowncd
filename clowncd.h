#ifndef CLOWNCD_H
#define CLOWNCD_H

#include "clowncommon/clowncommon.h"

#include "cue.h"
#include "file-io.h"

typedef struct ClownCD
{
	char *filename;
	ClownCD_File file;
	struct
	{
		ClownCD_File file;
		ClownCD_CueFileType file_type;
		ClownCD_CueTrackType type;
		unsigned long starting_sector, ending_sector;
	} track;
} ClownCD;

cc_bool ClownCD_OpenFromFile(ClownCD *disc, const char *file_path);
void ClownCD_Close(ClownCD *disc);

cc_bool ClownCD_SeekTrackIndex(ClownCD *disc, unsigned int track, unsigned int index);
cc_bool ClownCD_SeekSector(ClownCD *disc, unsigned long sector);

cc_bool ClownCD_ReadSectorRaw(ClownCD *disc, unsigned char *buffer);
cc_bool ClownCD_ReadSectorAtRaw(ClownCD *disc, unsigned long sector_index, unsigned char *buffer);
cc_bool ClownCD_ReadSectorData(ClownCD *disc, unsigned char *buffer);
cc_bool ClownCD_ReadSectorAtData(ClownCD *disc, unsigned long sector_index, unsigned char *buffer);

unsigned long ClownCD_CalculateSectorCRC(const unsigned char *buffer);
cc_bool ClownCD_ValidateSectorCRC(const unsigned char *buffer);

#endif /* CLOWNCD_H */
