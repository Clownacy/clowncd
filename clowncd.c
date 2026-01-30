#include "clowncd.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "cue.h"
#include "disc.h"
#include "utilities.h"

#define CLOWNCD_CRC_POLYNOMIAL 0xD8018001

static cc_bool ClownCD_IsFrameValid(ClownCD* const disc)
{
	return !(disc->track.current_frame < disc->track.starting_frame || disc->track.current_frame >= disc->track.starting_frame + disc->track.total_frames);
}

static cc_bool ClownCD_SeekFrameInternal(ClownCD* const disc, const size_t frame)
{
	const size_t header_size = disc->track.header_size;
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

void ClownCD_OpenAlreadyOpen(ClownCD *disc, void *stream, const char *file_path, const ClownCD_FileCallbacks *callbacks)
{
	disc->filename = ClownCD_DuplicateString(file_path); /* It's okay for this to fail. */
	disc->file = stream != NULL ? ClownCD_FileOpenAlreadyOpen(stream, callbacks) : ClownCD_FileOpen(file_path, CLOWNCD_RB, callbacks);

	disc->track.header_size = 0;
	disc->track.audio_decoder_needed = cc_false;
	disc->track.has_full_sized_sectors = cc_false;
	disc->track.starting_frame = 0;
	disc->track.current_frame = 0;
	disc->track.total_frames = 0;

	ClownCD_DiscOpen(disc);
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

static cc_bool ClownCD_SeekTrackIndexInternal(ClownCD* const disc, const unsigned int track, const unsigned int index)
{
	if (track != disc->track.current_track || index != disc->track.current_index)
	{
		disc->track.current_track = track;
		disc->track.current_index = index;

		if (!ClownCD_DiscSeekTrackIndex(disc, track, index))
			return cc_false;

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
