#include "clowncd.h"

#include <stdlib.h>

#include "cue.h"
#include "utilities.h"

#define CLOWNCD_SECTOR_RAW_SIZE 2352
#define CLOWNCD_SECTOR_HEADER_SIZE 0x10
#define CLOWNCD_SECTOR_DATA_SIZE 0x800

#define CLOWNCD_CRC_POLYNOMIAL 0xD8018001

cc_bool ClownCD_OpenFromFile(ClownCD* const disc, const char* const file_path)
{
	disc->filename = ClownCD_DuplicateString(file_path);

	if (disc->filename == NULL)
		return cc_false;

	disc->file = ClownCD_FileOpen(file_path, CLOWNCD_RB);
	disc->track.file.stream = NULL;

	return ClownCD_FileIsOpen(&disc->file);
}

void ClownCD_Close(ClownCD* const disc)
{
	if (ClownCD_FileIsOpen(&disc->track.file))
		ClownCD_FileClose(&disc->track.file);

	ClownCD_FileClose(&disc->file);
	free(disc->filename);
}

static void ClownCD_SeekTrackCallback(
	void* const user_data,
	const char* const filename,
	const ClownCD_CueFileType file_type,
	const unsigned int track,
	const ClownCD_CueTrackType track_type,
	const unsigned int index,
	const unsigned long frame)
{
	/* TODO: Cache the track filename so that we don't reopen files unnecessarily. */
	ClownCD* const disc = (ClownCD*)user_data;
	char* const full_path = ClownCD_GetFullFilePath(disc->filename, filename);

	(void)index;

	if (ClownCD_FileIsOpen(&disc->track.file))
		ClownCD_FileClose(&disc->track.file);

	if (full_path != NULL)
	{
		disc->track.file = ClownCD_FileOpen(full_path, CLOWNCD_RB);
		free(full_path);

		disc->track.file_type = file_type;
		disc->track.type = track_type;
		disc->track.starting_sector = frame;
		disc->track.ending_sector = ClownCD_CueGetTrackEndingFrame(&disc->file, filename, track, frame);
	}
}

cc_bool ClownCD_SeekTrackIndex(ClownCD* const disc, const unsigned int track, const unsigned int index)
{
	if (!ClownCD_CueGetTrackIndexInfo(&disc->file, track, index, ClownCD_SeekTrackCallback, disc))
		return cc_false;

	if (!ClownCD_SeekSector(disc, disc->track.starting_sector))
		return cc_false;

	return cc_true;
}

cc_bool ClownCD_SeekSector(ClownCD* const disc, const unsigned long sector)
{
	if (sector < disc->track.starting_sector || sector >= disc->track.ending_sector)
		return cc_false;

	if (ClownCD_FileSeek(&disc->track.file, (sector - disc->track.starting_sector) * CLOWNCD_SECTOR_RAW_SIZE, CLOWNCD_SEEK_SET) != 0)
		return cc_false;

	return cc_true;
}

static cc_bool ClownCD_ReadSectorAt(ClownCD* const disc, const unsigned long sector_index, unsigned char* const buffer, cc_bool (*callback)(ClownCD *disc, unsigned char *buffer))
{
	const long position = ClownCD_FileTell(&disc->track.file);

	cc_bool success = cc_false;

	if (position != -1L)
	{
		if (ClownCD_SeekSector(disc, sector_index))
			if (callback(disc, buffer))
				success = cc_true;

		if (ClownCD_FileSeek(&disc->track.file, position, CLOWNCD_SEEK_SET) != 0)
			success = cc_false;
	}

	return success;
}

cc_bool ClownCD_ReadSectorRaw(ClownCD* const disc, unsigned char* const buffer)
{
	return ClownCD_FileRead(buffer, CLOWNCD_SECTOR_RAW_SIZE, 1, &disc->track.file) == 1;
}

cc_bool ClownCD_ReadSectorAtRaw(ClownCD* const disc, const unsigned long sector_index, unsigned char* const buffer)
{
	return ClownCD_ReadSectorAt(disc, sector_index, buffer, ClownCD_ReadSectorRaw);
}

cc_bool ClownCD_ReadSectorData(ClownCD* const disc, unsigned char* const buffer)
{
	if (ClownCD_FileSeek(&disc->track.file, CLOWNCD_SECTOR_HEADER_SIZE, CLOWNCD_SEEK_CUR) != 0)
		return cc_false;

	return ClownCD_FileRead(buffer, CLOWNCD_SECTOR_DATA_SIZE, 1, &disc->track.file) == 1;
}

cc_bool ClownCD_ReadSectorAtData(ClownCD* const disc, const unsigned long sector_index, unsigned char* const buffer)
{
	return ClownCD_ReadSectorAt(disc, sector_index, buffer, ClownCD_ReadSectorData);
}

unsigned long ClownCD_CalculateSectorCRC(const unsigned char* const buffer)
{
	unsigned long shift, i;

	shift = 0;

	for (i = 0; i < (CLOWNCD_SECTOR_HEADER_SIZE + CLOWNCD_SECTOR_DATA_SIZE) * 8; ++i)
	{
		const unsigned int bit = i % 8;
		const unsigned int byte = i / 8;

		const unsigned long popped_bit = shift & 1;

		shift >>= 1;

		shift |= (unsigned long)((buffer[byte] >> bit) & 1) << 31;

		if (popped_bit != 0)
			shift ^= CLOWNCD_CRC_POLYNOMIAL;
	}

	for (i = 0; i < 32; ++i)
	{
		const unsigned long popped_bit = shift & 1;

		shift >>= 1;

		if (popped_bit != 0)
			shift ^= CLOWNCD_CRC_POLYNOMIAL;
	}

	return shift;
}

cc_bool ClownCD_ValidateSectorCRC(const unsigned char* const buffer)
{
	const unsigned long old_crc = ClownCD_Read32LEMemory(&buffer[CLOWNCD_SECTOR_HEADER_SIZE + CLOWNCD_SECTOR_DATA_SIZE]);
	const unsigned long new_crc = ClownCD_CalculateSectorCRC(buffer);

	return new_crc == old_crc;
}
