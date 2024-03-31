#include "clowncd.h"

#include <stdlib.h>

#include "cue.h"
#include "utilities.h"

#define CLOWNCD_SECTOR_RAW_SIZE 2352
#define CLOWNCD_SECTOR_HEADER_SIZE 0x10
#define CLOWNCD_SECTOR_DATA_SIZE 0x800
#define CLOWNCD_AUDIO_FRAME_SIZE (2 * 2)

#define CLOWNCD_CRC_POLYNOMIAL 0xD8018001

static cc_bool ClownCD_IsSectorValid(ClownCD* const disc)
{
	return !(disc->track.current_sector < disc->track.starting_sector || disc->track.current_sector >= disc->track.ending_sector);
}

static cc_bool ClownCD_SeekSectorInternal(ClownCD* const disc, const unsigned long sector)
{
	disc->track.current_sector = disc->track.starting_sector + sector;

	if (!ClownCD_IsSectorValid(disc))
		return cc_false;

	if (ClownCD_FileSeek(&disc->track.file, disc->track.current_sector * CLOWNCD_SECTOR_RAW_SIZE, CLOWNCD_SEEK_SET) != 0)
		return cc_false;

	return cc_true;
}

ClownCD ClownCD_Open(const char* const file_path, const ClownCD_FileCallbacks* const callbacks)
{
	return ClownCD_OpenAlreadyOpen(NULL, file_path, callbacks);
}

ClownCD ClownCD_OpenAlreadyOpen(void *stream, const char *file_path, const ClownCD_FileCallbacks *callbacks)
{
	ClownCD disc;

	disc.filename = ClownCD_DuplicateString(file_path); /* It's okay for this to fail. */
	disc.file = stream != NULL ? ClownCD_FileOpenAlreadyOpen(stream, callbacks) : ClownCD_FileOpen(file_path, CLOWNCD_RB, callbacks);
	disc.track.file = ClownCD_FileOpenBlank();
	disc.track.file_type = CLOWNCD_CUE_FILE_INVALID;
	disc.track.type = CLOWNCD_CUE_TRACK_INVALID;
	disc.track.current_audio_frame = 0;
	disc.track.total_audio_frames = 0;
	disc.track.starting_sector = 0;
	disc.track.ending_sector = 0;
	disc.track.current_sector = 0;

	return disc;
}

void ClownCD_Close(ClownCD* const disc)
{
	if (ClownCD_FileIsOpen(&disc->track.file))
		ClownCD_FileClose(&disc->track.file);

	if (ClownCD_FileIsOpen(&disc->file))
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
		disc->track.file = ClownCD_FileOpen(full_path, CLOWNCD_RB, disc->file.functions);
		free(full_path);

		disc->track.file_type = file_type;
		disc->track.type = track_type;
		disc->track.starting_sector = frame;
		disc->track.ending_sector = ClownCD_CueGetTrackEndingFrame(&disc->file, filename, track, frame);
		disc->track.current_audio_frame = 0;
		disc->track.total_audio_frames = (size_t)(disc->track.ending_sector - disc->track.starting_sector) * (CLOWNCD_SECTOR_RAW_SIZE / CLOWNCD_AUDIO_FRAME_SIZE);
	}
}

ClownCD_CueTrackType ClownCD_SeekTrackIndex(ClownCD* const disc, const unsigned int track, const unsigned int index)
{
	if (!ClownCD_CueGetTrackIndexInfo(&disc->file, track, index, ClownCD_SeekTrackCallback, disc))
		return CLOWNCD_CUE_TRACK_INVALID;

	if (!ClownCD_SeekSectorInternal(disc, 0))
		return CLOWNCD_CUE_TRACK_INVALID;

	return disc->track.type;
}

cc_bool ClownCD_SeekSector(ClownCD* const disc, const unsigned long sector)
{
	if (/*disc->track.type != CLOWNCD_CUE_TRACK_MODE1_2048 && */disc->track.type != CLOWNCD_CUE_TRACK_MODE1_2352)
		return cc_false;

	return ClownCD_SeekSectorInternal(disc, sector);
}

cc_bool ClownCD_SeekAudioFrame(ClownCD* const disc, const size_t frame)
{
	if (disc->track.type != CLOWNCD_CUE_TRACK_AUDIO)
		return cc_false;

	if (frame >= disc->track.total_audio_frames)
		return cc_false;

	/* Seek to the start of the track. */
	if (!ClownCD_SeekSectorInternal(disc, 0))
		return cc_false;

	disc->track.current_audio_frame = frame;

	/* Seek to the correct frame within the track. */
	if (ClownCD_FileSeek(&disc->track.file, disc->track.current_audio_frame * CLOWNCD_AUDIO_FRAME_SIZE, CLOWNCD_SEEK_CUR) != 0)
		return cc_false;

	return cc_true;
}

static cc_bool ClownCD_ReadSectorAt(ClownCD* const disc, const unsigned long sector_index, unsigned char* const buffer, cc_bool (*callback)(ClownCD *disc, unsigned char *buffer))
{
	cc_bool success = cc_false;

	if (/*disc->track.type == CLOWNCD_CUE_TRACK_MODE1_2048 || */disc->track.type == CLOWNCD_CUE_TRACK_MODE1_2352)
	{
		const long position = ClownCD_FileTell(&disc->track.file);

		if (position != -1L)
		{
			if (ClownCD_SeekSector(disc, sector_index))
				if (callback(disc, buffer))
					success = cc_true;

			if (ClownCD_FileSeek(&disc->track.file, position, CLOWNCD_SEEK_SET) != 0)
				success = cc_false;
		}
	}

	return success;
}

cc_bool ClownCD_ReadSectorRaw(ClownCD* const disc, unsigned char* const buffer)
{
	if (!ClownCD_IsSectorValid(disc))
		return cc_false;

	++disc->track.current_sector;

	return ClownCD_FileRead(buffer, CLOWNCD_SECTOR_RAW_SIZE, 1, &disc->track.file) == 1;
}

cc_bool ClownCD_ReadSectorAtRaw(ClownCD* const disc, const unsigned long sector_index, unsigned char* const buffer)
{
	return ClownCD_ReadSectorAt(disc, sector_index, buffer, ClownCD_ReadSectorRaw);
}

cc_bool ClownCD_ReadSectorData(ClownCD* const disc, unsigned char* const buffer)
{
	if (!ClownCD_IsSectorValid(disc))
		return cc_false;

	++disc->track.current_sector;

	if (ClownCD_FileSeek(&disc->track.file, CLOWNCD_SECTOR_HEADER_SIZE, CLOWNCD_SEEK_CUR) != 0)
		return cc_false;

	if (ClownCD_FileRead(buffer, CLOWNCD_SECTOR_DATA_SIZE, 1, &disc->track.file) != 1)
		return cc_false;

	if (ClownCD_FileSeek(&disc->track.file, CLOWNCD_SECTOR_RAW_SIZE - (CLOWNCD_SECTOR_HEADER_SIZE + CLOWNCD_SECTOR_DATA_SIZE), CLOWNCD_SEEK_CUR) != 0)
		return cc_false;

	return cc_true;
}

cc_bool ClownCD_ReadSectorAtData(ClownCD* const disc, const unsigned long sector_index, unsigned char* const buffer)
{
	return ClownCD_ReadSectorAt(disc, sector_index, buffer, ClownCD_ReadSectorData);
}

size_t ClownCD_ReadAudioFrames(ClownCD* const disc, short* const buffer, const size_t total_frames)
{
	const size_t frames_to_do = CC_MIN(disc->track.total_audio_frames - disc->track.current_audio_frame, total_frames);

	short *buffer_pointer = buffer;
	size_t i;

	if (disc->track.type != CLOWNCD_CUE_TRACK_AUDIO)
		return 0;

	for (i = 0; i < frames_to_do; ++i)
	{
		*buffer_pointer++ = ClownCD_ReadS16LE(&disc->track.file);
		*buffer_pointer++ = ClownCD_ReadS16LE(&disc->track.file);

		if (disc->track.file.eof)
			break;
	}

	disc->track.current_audio_frame += frames_to_do;

	return i;
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
	const unsigned long old_crc = ClownCD_ReadU32LEMemory(&buffer[CLOWNCD_SECTOR_HEADER_SIZE + CLOWNCD_SECTOR_DATA_SIZE]);
	const unsigned long new_crc = ClownCD_CalculateSectorCRC(buffer);

	return new_crc == old_crc;
}
