#include "clowncd.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "cue.h"
#include "utilities.h"

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

static cc_bool ClownCD_IsFrameValid(ClownCD* const disc)
{
	return !(disc->track.current_frame < disc->track.starting_frame || disc->track.current_frame >= disc->track.starting_frame + disc->track.total_frames);
}

static cc_bool ClownCD_SeekFrameInternal(ClownCD* const disc, const size_t frame)
{
	const size_t header_size = ClownCD_GetHeaderSize(disc);
	const size_t sector_size = disc->track.has_full_sized_sectors ? CLOWNCD_SECTOR_RAW_SIZE : CLOWNCD_SECTOR_DATA_SIZE;

	if (header_size == (size_t)-1)
		return cc_false;

	disc->track.current_frame = disc->track.starting_frame + frame;

	if (!ClownCD_IsFrameValid(disc))
		return cc_false;

	if (ClownCD_FileSeek(&disc->track.file, header_size + disc->track.current_frame / CLOWNCD_AUDIO_FRAMES_PER_SECTOR * sector_size + disc->track.current_frame % CLOWNCD_AUDIO_FRAMES_PER_SECTOR * CLOWNCD_AUDIO_FRAME_SIZE, CLOWNCD_SEEK_SET) != 0)
		return cc_false;

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

	disc.track.audio_decoder_needed = cc_false;
	disc.track.has_full_sized_sectors = cc_false;
	disc.track.starting_frame = 0;
	disc.track.current_frame = 0;
	disc.track.total_frames = 0;

	return disc;
}

void ClownCD_Close(ClownCD* const disc)
{
	if (ClownCD_FileIsOpen(&disc->track.file))
	{
		if (disc->track.audio_decoder_needed)
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
		if (disc->track.audio_decoder_needed)
			ClownCD_AudioClose(&disc->track.audio);

		ClownCD_FileClose(&disc->track.file);
	}
}

static void ClownCD_SeekTrackIndexCallback(
	void* const user_data,
	const char* const filename,
	const ClownCD_CueFileType file_type,
	const unsigned int track,
	const ClownCD_CueTrackType track_type,
	const unsigned int index,
	const unsigned long sector)
{
	/* TODO: Cache the track filename so that we don't reopen files unnecessarily. */
	ClownCD* const disc = (ClownCD*)user_data;
	char* const full_path = ClownCD_GetFullFilePath(disc->filename, filename);

	ClownCD_CloseTrackFile(disc);

	if (full_path != NULL)
	{
		disc->track.file = ClownCD_FileOpen(full_path, CLOWNCD_RB, disc->file.functions);
		free(full_path);

		disc->track.audio_decoder_needed = file_type == CLOWNCD_CUE_FILE_WAVE || file_type == CLOWNCD_CUE_FILE_MP3;
		disc->track.has_full_sized_sectors = track_type != CLOWNCD_CUE_TRACK_MODE1_2048;
		disc->track.starting_frame = sector * CLOWNCD_AUDIO_FRAMES_PER_SECTOR;
		disc->track.total_frames = ClownCD_CueGetTrackIndexEndingSector(&disc->file, filename, track, index, sector) * CLOWNCD_AUDIO_FRAMES_PER_SECTOR - disc->track.starting_frame;

		if (disc->track.audio_decoder_needed)
			if (!ClownCD_AudioOpen(&disc->track.audio, &disc->track.file))
				ClownCD_FileClose(&disc->track.file);
	}
}

static unsigned int ClownCD_GetClownCDTrackSectorSize(const unsigned int value)
{
	switch (value)
	{
		case 0:
		case 1:
		default:
			return CLOWNCD_SECTOR_RAW_SIZE;
	}
}

static cc_bool ClownCD_SeekTrackIndexInternal(ClownCD* const disc, const unsigned int track, const unsigned int index)
{
	if (track != disc->track.current_track || index != disc->track.current_index)
	{
		disc->track.current_track = track;
		disc->track.current_index = index;

		switch (disc->type)
		{
			case CLOWNCD_DISC_CUE:
				if (!ClownCD_CueGetTrackIndexInfo(&disc->file, track, index, ClownCD_SeekTrackIndexCallback, disc))
					return cc_false;

				if (!ClownCD_FileIsOpen(&disc->track.file))
					return cc_false;

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

					disc->track.audio_decoder_needed = cc_false;
					disc->track.has_full_sized_sectors = disc->type != CLOWNCD_DISC_RAW_2048;
					disc->track.starting_frame = 0;
					disc->track.total_frames = -1;
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

					disc->track.audio_decoder_needed = cc_true;
					disc->track.has_full_sized_sectors = cc_true;
					disc->track.starting_frame = 0;
					disc->track.total_frames = -1;

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

				disc->track.audio_decoder_needed = cc_false;
				disc->track.has_full_sized_sectors = cc_true;
				ClownCD_ReadU16BE(&disc->track.file); /* The unused track type value. */
				disc->track.starting_frame = ClownCD_ReadU32BE(&disc->track.file) * CLOWNCD_AUDIO_FRAMES_PER_SECTOR;
				disc->track.total_frames = ClownCD_ReadU32BE(&disc->track.file) * CLOWNCD_AUDIO_FRAMES_PER_SECTOR;
				break;
		}

		/* Force the frame to update. */
		disc->track.current_frame = -1;
	}

	return cc_true;
}

cc_bool ClownCD_SeekAudioFrame(ClownCD* const disc, const size_t frame)
{
	if (frame >= disc->track.total_frames)
		return cc_false;

	if (frame != disc->track.current_frame)
	{
		disc->track.current_frame = frame;

		if (disc->track.audio_decoder_needed)
		{
			if (!ClownCD_AudioSeek(&disc->track.audio, disc->track.starting_frame + frame))
				return cc_false;
		}
		else
		{
			if (!ClownCD_SeekFrameInternal(disc, disc->track.current_frame))
				return cc_false;
		}
	}

	return cc_true;
}

cc_bool ClownCD_SetState(ClownCD* const disc, const unsigned int track, const unsigned int index, const size_t frame)
{
	if (!ClownCD_SeekTrackIndexInternal(disc, track, index))
		return cc_false;

	if (!ClownCD_SeekAudioFrame(disc, frame))
		return cc_false;

	return cc_true;
}

cc_bool ClownCD_BeginSectorStream(ClownCD* const disc)
{
	if (!ClownCD_IsFrameValid(disc))
		return cc_false;

	disc->track.current_frame += CLOWNCD_AUDIO_FRAMES_PER_SECTOR;

	/* Skip header. */
	if (disc->track.has_full_sized_sectors)
		if (ClownCD_FileSeek(&disc->track.file, CLOWNCD_SECTOR_HEADER_SIZE, CLOWNCD_SEEK_CUR) != 0)
			return cc_false;

	return cc_true;
}

size_t ClownCD_ReadSectorStream(ClownCD* const disc, unsigned char* const buffer, const size_t total_bytes)
{
	return ClownCD_FileRead(buffer, 1, total_bytes, &disc->track.file);
}

cc_bool ClownCD_EndSectorStream(ClownCD* const disc)
{
	/* Skip error-correction data. */
	if (disc->track.has_full_sized_sectors)
		if (ClownCD_FileSeek(&disc->track.file, CLOWNCD_SECTOR_RAW_SIZE - (CLOWNCD_SECTOR_HEADER_SIZE + CLOWNCD_SECTOR_DATA_SIZE), CLOWNCD_SEEK_CUR) != 0)
			return cc_false;

	return cc_true;
}

size_t ClownCD_ReadSector(ClownCD* const disc, unsigned char* const buffer)
{
	size_t bytes_read;

	if (!ClownCD_BeginSectorStream(disc))
		return 0;

	bytes_read = ClownCD_ReadSectorStream(disc, buffer, CLOWNCD_SECTOR_DATA_SIZE);

	ClownCD_EndSectorStream(disc);

	return bytes_read;
}

static size_t ClownCD_ReadFramesGetAudio(ClownCD* const disc, short* const buffer, const size_t total_frames)
{
	const size_t frames_to_do = CC_MIN(disc->track.total_frames - disc->track.current_frame, total_frames);

	size_t frames_done;

	if (disc->track.audio_decoder_needed)
	{
		frames_done = ClownCD_AudioRead(&disc->track.audio, buffer, frames_to_do);
	}
	else
	{
		short *buffer_pointer = buffer;

		if (!disc->track.has_full_sized_sectors)
			return 0;

		for (frames_done = 0; frames_done < frames_to_do; ++frames_done)
		{
			*buffer_pointer++ = ClownCD_ReadS16LE(&disc->track.file);
			*buffer_pointer++ = ClownCD_ReadS16LE(&disc->track.file);

			if (disc->track.file.eof)
				break;
		}
	}

	disc->track.current_frame += frames_done;

	return frames_done;
}

static size_t ClownCD_ReadFramesGeneratePadding(ClownCD* const disc, short* const buffer, const size_t total_frames)
{
	const size_t occupied_frames_in_sector = disc->track.current_frame % CLOWNCD_AUDIO_FRAMES_PER_SECTOR;
	const size_t empty_frames_in_sector = occupied_frames_in_sector == 0 ? 0 : CLOWNCD_AUDIO_FRAMES_PER_SECTOR - occupied_frames_in_sector;

	const size_t frames_to_do = CC_MIN(empty_frames_in_sector, total_frames);

	memset(buffer, 0, frames_to_do * CLOWNCD_AUDIO_FRAME_SIZE);

	return frames_to_do;
}

size_t ClownCD_ReadFrames(ClownCD* const disc, short* const buffer, const size_t total_frames)
{
	const size_t audio_frames_done = ClownCD_ReadFramesGetAudio(disc, buffer, total_frames);
	const size_t padding_frames_done = ClownCD_ReadFramesGeneratePadding(disc, buffer + audio_frames_done * CLOWNCD_AUDIO_CHANNELS, total_frames - audio_frames_done);

	return audio_frames_done + padding_frames_done;
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
