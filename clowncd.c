#include "clowncd.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "cue.h"
#include "utilities.h"
#include "vorbis.h"
#include "wav.h"

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
	disc.track.starting_frame = 0;
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
	{
		if (disc->track.file_type == CLOWNCD_CUE_FILE_WAVE || disc->track.file_type == CLOWNCD_CUE_FILE_MP3)
			ClownCD_AudioClose(&disc->track.audio);

		ClownCD_FileClose(&disc->track.file);
	}

	if (ClownCD_FileIsOpen(&disc->file))
		ClownCD_FileClose(&disc->file);

	free(disc->filename);
}

static void ClownCD_CloseTrackFile(ClownCD* const disc)
{
	if (ClownCD_FileIsOpen(&disc->track.file))
	{
		if (disc->track.file_type == CLOWNCD_CUE_FILE_WAVE || disc->track.file_type == CLOWNCD_CUE_FILE_MP3)
			ClownCD_AudioClose(&disc->track.audio);

		ClownCD_FileClose(&disc->track.file);
	}
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

	ClownCD_CloseTrackFile(disc);

	if (full_path != NULL)
	{
		disc->track.file = ClownCD_FileOpen(full_path, CLOWNCD_RB, disc->file.functions);
		free(full_path);

		disc->track.file_type = file_type;
		disc->track.type = track_type;
		disc->track.starting_sector = frame;
		disc->track.ending_sector = ClownCD_CueGetTrackEndingFrame(&disc->file, filename, track, frame);

		if (disc->track.file_type == CLOWNCD_CUE_FILE_WAVE || disc->track.file_type == CLOWNCD_CUE_FILE_MP3)
			if (!ClownCD_AudioOpen(&disc->track.audio, &disc->track.file))
				ClownCD_FileClose(&disc->track.file);
	}
}

static ClownCD_CueTrackType ClownCD_GetClownCDTrackType(const unsigned int value)
{
	switch (value)
	{
		case 0:
			return CLOWNCD_CUE_TRACK_MODE1_2352;

		case 1:
			return CLOWNCD_CUE_TRACK_AUDIO;

		default:
			return CLOWNCD_CUE_TRACK_INVALID;
	}
}

static size_t ClownCD_SectorToFrame(const unsigned long sector)
{
	const size_t frames_per_second = 75;
	const size_t sample_rate = 44100;
	const size_t frame = sector / frames_per_second * sample_rate + sector % frames_per_second * sample_rate / frames_per_second;
	return frame;
}

static cc_bool ClownCD_SeekTrackIndexInternal(ClownCD* const disc, const unsigned int track, const unsigned int index)
{
	if (track != disc->track.current_track || index != disc->track.current_index)
	{
		disc->track.current_track = track;
		disc->track.current_index = index;

		disc->track.type = CLOWNCD_CUE_TRACK_INVALID;

		switch (disc->type)
		{
			case CLOWNCD_DISC_CUE:
				if (!ClownCD_CueGetTrackIndexInfo(&disc->file, track, index, ClownCD_SeekTrackCallback, disc))
					return cc_false;

				if (!ClownCD_FileIsOpen(&disc->track.file))
				{
					disc->track.type = CLOWNCD_CUE_TRACK_INVALID;
					return cc_false;
				}

				break;

			case CLOWNCD_DISC_RAW_2048:
			case CLOWNCD_DISC_RAW_2352:
				if (index != 1)
					return cc_false;

				if (track == 1)
				{
					/* Make the disc file the active track file. */
					if (ClownCD_FileIsOpen(&disc->file))
					{
						disc->track.file = disc->file;
						disc->file = ClownCD_FileOpenBlank();
					}

					disc->track.file_type = CLOWNCD_CUE_FILE_BINARY;
					disc->track.type = disc->type == CLOWNCD_DISC_RAW_2048 ? CLOWNCD_CUE_TRACK_MODE1_2048 : CLOWNCD_CUE_TRACK_MODE1_2352;
					disc->track.starting_sector = 0;
					disc->track.ending_sector = 0xFFFFFFFF;
				}
				else if (track <= 99 && disc->filename != NULL)
				{
					const char extensions[][4] = {
						{'F', 'L', 'A', 'C'},
						{'f', 'l', 'a', 'c'},
						{'M', 'P', '3', '\0'},
						{'m', 'p', '3', '\0'},
						{'O', 'G', 'G', '\0'},
						{'o', 'g', 'g', '\0'},
						{'W', 'A', 'V', '\0'},
						{'w', 'a', 'v', '\0'},
					};
					const char* const file_extension = ClownCD_GetFileExtension(disc->filename);
					const size_t filename_length_minus_extension = file_extension == NULL ? strlen(disc->filename) : (size_t)(file_extension - disc->filename);
					char* const audio_filename = (char*)malloc(filename_length_minus_extension + 4 + sizeof(extensions[0]) + 1);

					size_t i;

					/* Make the disc file not the active track file. */
					if (!ClownCD_FileIsOpen(&disc->file))
					{
						disc->file = disc->track.file;
						disc->track.file = ClownCD_FileOpenBlank();
					}

					ClownCD_CloseTrackFile(disc);

					disc->track.file_type = CLOWNCD_CUE_FILE_WAVE;
					disc->track.type = CLOWNCD_CUE_TRACK_AUDIO;
					disc->track.starting_sector = 0;
					disc->track.ending_sector = 0xFFFFFFFF;

					if (audio_filename == NULL)
						return cc_false;

					memcpy(audio_filename, disc->filename, filename_length_minus_extension);
					audio_filename[filename_length_minus_extension + 0] = ' ';
					audio_filename[filename_length_minus_extension + 1] = '0' + track / 10;
					audio_filename[filename_length_minus_extension + 2] = '0' + track % 10;
					audio_filename[filename_length_minus_extension + 3] = '.';
					audio_filename[filename_length_minus_extension + 4 + sizeof(extensions[0])] = '\0';

					for (i = 0; i < CC_COUNT_OF(extensions); ++i)
					{
						const char* const extension = extensions[i];

						memcpy(&audio_filename[filename_length_minus_extension + 4], extension, sizeof(extensions[i]));

						disc->track.file = ClownCD_FileOpen(audio_filename, CLOWNCD_RB, disc->file.functions);

						if (!ClownCD_AudioOpen(&disc->track.audio, &disc->track.file))
							ClownCD_FileClose(&disc->track.file);
						else
							break;
					}

					free(audio_filename);
				}
				else
				{
					return cc_false;
				}

				break;

			case CLOWNCD_DISC_CLOWNCD:
				if (index != 1)
					return cc_false;

				if (ClownCD_FileSeek(&disc->track.file, 10, CLOWNCD_SEEK_SET) != 0)
					return cc_false;

				if (track >= ClownCD_ReadU32BE(&disc->track.file))
					return cc_false;

				if (ClownCD_FileSeek(&disc->track.file, ClownCD_ClownCDTrackMetadataOffset(track), CLOWNCD_SEEK_SET) != 0)
					return cc_false;

				disc->track.file_type = CLOWNCD_CUE_FILE_BINARY;
				disc->track.type = ClownCD_GetClownCDTrackType(ClownCD_ReadU16BE(&disc->track.file));
				disc->track.starting_sector = ClownCD_ReadU32BE(&disc->track.file);
				disc->track.ending_sector = disc->track.starting_sector + ClownCD_ReadU32BE(&disc->track.file);
				break;
		}

		disc->track.starting_frame = ClownCD_SectorToFrame(disc->track.starting_sector);
		disc->track.total_frames = ClownCD_SectorToFrame(disc->track.ending_sector - disc->track.starting_sector);

		/* Force the sector and frame to update. */
		disc->track.current_sector = -1;
		disc->track.current_frame = -1;
	}

	return cc_true;
}

static cc_bool ClownCD_SeekSectorOrFrame(ClownCD* const disc, const unsigned long sector, const size_t frame)
{
	switch (disc->track.type)
	{
		case CLOWNCD_CUE_TRACK_INVALID:
			break;

		case CLOWNCD_CUE_TRACK_MODE1_2048:
		case CLOWNCD_CUE_TRACK_MODE1_2352:
			if (!ClownCD_SeekSector(disc, sector))
				return cc_false;
			break;

		case CLOWNCD_CUE_TRACK_AUDIO:
			if (!ClownCD_SeekAudioFrame(disc, frame))
				return cc_false;
			break;
	}

	return cc_true;
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

		switch (disc->track.file_type)
		{
			case CLOWNCD_CUE_FILE_BINARY:
				/* Seek to the start of the track. */
				if (!ClownCD_SeekSectorInternal(disc, 0))
					return cc_false;

				/* Seek to the correct frame within the track. */
				if (ClownCD_FileSeek(&disc->track.file, disc->track.current_frame * CLOWNCD_AUDIO_FRAME_SIZE, CLOWNCD_SEEK_CUR) != 0)
					return cc_false;

				break;

			case CLOWNCD_CUE_FILE_WAVE:
			case CLOWNCD_CUE_FILE_MP3:
				if (!ClownCD_AudioSeek(&disc->track.audio, disc->track.starting_frame + frame))
					return cc_false;
				break;

			default:
				assert(cc_false);
				return cc_false;
		}
	}

	return cc_true;
}

ClownCD_CueTrackType ClownCD_SetState(ClownCD* const disc, const unsigned int track, const unsigned int index, const unsigned long sector, const size_t frame)
{
	if (!ClownCD_SeekTrackIndexInternal(disc, track, index))
		return CLOWNCD_CUE_TRACK_INVALID;

	if (!ClownCD_SeekSectorOrFrame(disc, sector, frame))
		return CLOWNCD_CUE_TRACK_INVALID;

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

	size_t frames_done;

	switch (disc->track.file_type)
	{
		case CLOWNCD_CUE_FILE_BINARY:
		{
			short *buffer_pointer = buffer;

			if (disc->track.type != CLOWNCD_CUE_TRACK_AUDIO)
				return 0;

			for (frames_done = 0; frames_done < frames_to_do; ++frames_done)
			{
				*buffer_pointer++ = ClownCD_ReadS16LE(&disc->track.file);
				*buffer_pointer++ = ClownCD_ReadS16LE(&disc->track.file);

				if (disc->track.file.eof)
					break;
			}

			break;
		}

		case CLOWNCD_CUE_FILE_WAVE:
		case CLOWNCD_CUE_FILE_MP3:
			frames_done = ClownCD_AudioRead(&disc->track.audio, buffer, frames_to_do);
			break;

		default:
			assert(cc_false);
			frames_done = 0;
			break;
	}

	disc->track.current_frame += frames_done;

	return frames_done;
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
