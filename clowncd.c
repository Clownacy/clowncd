#include "clowncd.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "cue.h"
#include "utilities.h"

#define CLOWNCD_SECTOR_RAW_SIZE 2352
#define CLOWNCD_SECTOR_HEADER_SIZE 0x10
#define CLOWNCD_SECTOR_DATA_SIZE 0x800
#define CLOWNCD_AUDIO_FRAME_SIZE (2 * 2)

#define CLOWNCD_CRC_POLYNOMIAL 0xD8018001

static size_t ClownCD_ClownCDTrackMetadataOffset(const unsigned int track)
{
	assert(track != 0);
	return 12 + (track - 1) * 10;
}

static size_t ClownCD_GetHeaderSize(ClownCD* const disc)
{
	if (disc->type != CLOWNCD_DISC_CLOWNCD)
		return 0;

	if (ClownCD_FileSeek(&disc->track.file, 10, CLOWNCD_SEEK_SET) != 0)
		return -1;

	return ClownCD_ClownCDTrackMetadataOffset(ClownCD_ReadU16BE(&disc->track.file) + 1);
}

static cc_bool ClownCD_IsSectorValid(ClownCD* const disc)
{
	return !(disc->track.current_sector < disc->track.starting_sector || disc->track.current_sector >= disc->track.ending_sector);
}

static cc_bool ClownCD_SeekSectorInternal(ClownCD* const disc, const unsigned long sector)
{
	if (sector != disc->track.current_sector)
	{
		const size_t header_size = ClownCD_GetHeaderSize(disc);
		const size_t sector_size = disc->track.type == CLOWNCD_CUE_TRACK_MODE1_2048 ? CLOWNCD_SECTOR_DATA_SIZE : CLOWNCD_SECTOR_RAW_SIZE;

		if (header_size == (size_t)-1)
			return cc_false;

		disc->track.current_sector = disc->track.starting_sector + sector;

		if (!ClownCD_IsSectorValid(disc))
			return cc_false;

		if (ClownCD_FileSeek(&disc->track.file, header_size + disc->track.current_sector * sector_size, CLOWNCD_SEEK_SET) != 0)
			return cc_false;
	}

	return cc_true;
}

static ClownCD_DiscType ClownCD_GetDiscType(ClownCD_File* const file)
{
	static const unsigned char header_2048[0x10] = {0x53, 0x45, 0x47, 0x41, 0x44, 0x49, 0x53, 0x43, 0x53, 0x59, 0x53, 0x54, 0x45, 0x4D, 0x20, 0x20};
	static const unsigned char header_2352[0x10] = {0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x02, 0x00, 0x01};
	static const unsigned char header_clowncd_v0[0xA] = {0x63, 0x6C, 0x6F, 0x77, 0x6E, 0x63, 0x64, 0x00, 0x00, 0x00};

	unsigned char buffer[0x10];

	const cc_bool read_successful = ClownCD_FileRead(buffer, 0x10, 1, file) == 1;

	if (read_successful && memcmp(buffer, header_2048, sizeof(header_2048)) == 0)
		return CLOWNCD_DISC_RAW_2048;
	else if (read_successful && memcmp(buffer, header_2352, sizeof(header_2352)) == 0)
		return CLOWNCD_DISC_RAW_2352;
	else if (read_successful && memcmp(buffer, header_clowncd_v0, sizeof(header_clowncd_v0)) == 0)
		return CLOWNCD_DISC_CLOWNCD;
	else
		return CLOWNCD_DISC_CUE;
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
	disc.type = ClownCD_GetDiscType(&disc.file);

	switch (disc.type)
	{
		default:
			assert(cc_false);
			/* Fallthrough */
		case CLOWNCD_DISC_CUE:
			disc.track.file = ClownCD_FileOpenBlank();
			break;

		case CLOWNCD_DISC_RAW_2048:
		case CLOWNCD_DISC_RAW_2352:
		case CLOWNCD_DISC_CLOWNCD:
			disc.track.file = disc.file;
			disc.file = ClownCD_FileOpenBlank();
			break;
	}

	disc.track.file_type = CLOWNCD_CUE_FILE_INVALID;
	disc.track.type = CLOWNCD_CUE_TRACK_INVALID;
	disc.track.current_frame = 0;
	disc.track.total_frames = 0;
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
	}
}

ClownCD_CueTrackType ClownCD_SeekTrackIndex(ClownCD* const disc, const unsigned int track, const unsigned int index)
{
	return ClownCD_SetState(disc, track, index, 0, 0);
}

cc_bool ClownCD_SeekSector(ClownCD* const disc, const unsigned long sector)
{
	if (disc->track.type != CLOWNCD_CUE_TRACK_MODE1_2048 && disc->track.type != CLOWNCD_CUE_TRACK_MODE1_2352)
		return cc_false;

	if (!ClownCD_SeekSectorInternal(disc, sector))
		return cc_false;

	return cc_true;
}

cc_bool ClownCD_SeekAudioFrame(ClownCD* const disc, const size_t frame)
{
	if (disc->track.type != CLOWNCD_CUE_TRACK_AUDIO)
		return cc_false;

	if (frame >= disc->track.total_frames)
		return cc_false;

	if (frame != disc->track.current_frame)
	{
		disc->track.current_frame = frame;

		/* Seek to the start of the track. */
		if (!ClownCD_SeekSectorInternal(disc, 0))
			return cc_false;

		/* Seek to the correct frame within the track. */
		if (ClownCD_FileSeek(&disc->track.file, disc->track.current_frame * CLOWNCD_AUDIO_FRAME_SIZE, CLOWNCD_SEEK_CUR) != 0)
			return cc_false;
	}

	return cc_true;
}

static ClownCD_CueTrackType ClownCD_GetClownCDTrackType(const unsigned int value)
{
	ClownCD_CueTrackType track_type;

	switch (value)
	{
		case 0:
			track_type = CLOWNCD_CUE_TRACK_MODE1_2352;
			break;

		case 1:
			track_type = CLOWNCD_CUE_TRACK_AUDIO;
			break;

		default:
			track_type = CLOWNCD_CUE_TRACK_INVALID;
			break;
	}

	return track_type;
}

ClownCD_CueTrackType ClownCD_SetState(ClownCD* const disc, const unsigned int track, const unsigned int index, const unsigned long sector, const size_t frame)
{
	if (track != disc->track.current_track || index != disc->track.current_index)
	{
		disc->track.current_track = track;
		disc->track.current_index = index;

		switch (disc->type)
		{
			case CLOWNCD_DISC_CUE:
				if (!ClownCD_CueGetTrackIndexInfo(&disc->file, track, index, ClownCD_SeekTrackCallback, disc))
					return CLOWNCD_CUE_TRACK_INVALID;
				break;

			case CLOWNCD_DISC_RAW_2048:
			case CLOWNCD_DISC_RAW_2352:
				if (track == 1 && index == 1)
				{
					disc->track.file_type = CLOWNCD_CUE_FILE_BINARY;
					disc->track.type = disc->type == CLOWNCD_DISC_RAW_2048 ? CLOWNCD_CUE_TRACK_MODE1_2048 : CLOWNCD_CUE_TRACK_MODE1_2352;
					disc->track.starting_sector = 0;
					disc->track.ending_sector = 0xFFFFFFFF;
				}
				break;

			case CLOWNCD_DISC_CLOWNCD:
			{
				if (ClownCD_FileSeek(&disc->track.file, 10, CLOWNCD_SEEK_SET) != 0)
					return CLOWNCD_CUE_TRACK_INVALID;

				if (track >= ClownCD_ReadU32BE(&disc->track.file))
					return CLOWNCD_CUE_TRACK_INVALID;

				if (ClownCD_FileSeek(&disc->track.file, ClownCD_ClownCDTrackMetadataOffset(track), CLOWNCD_SEEK_SET) != 0)
					return CLOWNCD_CUE_TRACK_INVALID;

				disc->track.file_type = CLOWNCD_CUE_FILE_BINARY;
				disc->track.type = ClownCD_GetClownCDTrackType(ClownCD_ReadU16BE(&disc->track.file));
				disc->track.starting_sector = ClownCD_ReadU32BE(&disc->track.file);
				disc->track.ending_sector = disc->track.starting_sector + ClownCD_ReadU32BE(&disc->track.file);
				break;
			}
		}

		disc->track.total_frames = (size_t)(disc->track.ending_sector - disc->track.starting_sector) * (CLOWNCD_SECTOR_RAW_SIZE / CLOWNCD_AUDIO_FRAME_SIZE);

		if (!ClownCD_FileIsOpen(&disc->track.file))
			return CLOWNCD_CUE_TRACK_INVALID;

		/* Force the sector and frame to update. */
		disc->track.current_sector = -1;
		disc->track.current_frame = -1;
	}

	switch (disc->track.type)
	{
		case CLOWNCD_CUE_TRACK_INVALID:
			break;

		case CLOWNCD_CUE_TRACK_MODE1_2048:
		case CLOWNCD_CUE_TRACK_MODE1_2352:
			if (!ClownCD_SeekSector(disc, sector))
				return CLOWNCD_CUE_TRACK_INVALID;
			break;

		case CLOWNCD_CUE_TRACK_AUDIO:
			if (!ClownCD_SeekAudioFrame(disc, frame))
				return CLOWNCD_CUE_TRACK_INVALID;
			break;
	}

	return disc->track.type;
}

cc_bool ClownCD_ReadSector(ClownCD* const disc, unsigned char* const buffer)
{
	if (disc->track.type != CLOWNCD_CUE_TRACK_MODE1_2048 && disc->track.type != CLOWNCD_CUE_TRACK_MODE1_2352)
		return cc_false;

	if (!ClownCD_IsSectorValid(disc))
		return cc_false;

	++disc->track.current_sector;

	if (disc->track.type == CLOWNCD_CUE_TRACK_MODE1_2352)
		if (ClownCD_FileSeek(&disc->track.file, CLOWNCD_SECTOR_HEADER_SIZE, CLOWNCD_SEEK_CUR) != 0)
			return cc_false;

	if (ClownCD_FileRead(buffer, CLOWNCD_SECTOR_DATA_SIZE, 1, &disc->track.file) != 1)
		return cc_false;

	if (disc->track.type == CLOWNCD_CUE_TRACK_MODE1_2352)
		if (ClownCD_FileSeek(&disc->track.file, CLOWNCD_SECTOR_RAW_SIZE - (CLOWNCD_SECTOR_HEADER_SIZE + CLOWNCD_SECTOR_DATA_SIZE), CLOWNCD_SEEK_CUR) != 0)
			return cc_false;

	return cc_true;
}

size_t ClownCD_ReadFrames(ClownCD* const disc, short* const buffer, const size_t total_frames)
{
	const size_t frames_to_do = CC_MIN(disc->track.total_frames - disc->track.current_frame, total_frames);

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

	disc->track.current_frame += frames_to_do;

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
