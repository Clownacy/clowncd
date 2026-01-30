#include "clowncd.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "cue.h"
#include "utilities.h"

#define CLOWNCD_CRC_POLYNOMIAL 0xD8018001

static cc_bool ClownCD_IsFrameValid(ClownCD* const state)
{
	return !(state->disc.track.current_frame < state->disc.track.starting_frame || state->disc.track.current_frame >= state->disc.track.starting_frame + state->disc.track.total_frames);
}

static cc_bool ClownCD_SeekFrameInternal(ClownCD* const state, const size_t frame)
{
	const size_t header_size = state->disc.track.header_size;
	const size_t sector_size = state->disc.track.has_full_sized_sectors ? CLOWNCD_SECTOR_RAW_SIZE : CLOWNCD_SECTOR_DATA_SIZE;

	if (header_size == (size_t)-1)
		return cc_false;

	state->disc.track.current_frame = state->disc.track.starting_frame + frame;

	if (!ClownCD_IsFrameValid(state))
		return cc_false;

	if (ClownCD_FileSeek(&state->disc.track.file, header_size + state->disc.track.current_frame / CLOWNCD_AUDIO_FRAMES_PER_SECTOR * sector_size + state->disc.track.current_frame % CLOWNCD_AUDIO_FRAMES_PER_SECTOR * CLOWNCD_AUDIO_FRAME_SIZE, CLOWNCD_SEEK_SET) != 0)
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
	/* TODO: This uses too much stack!!! */
	ClownCD state;

	state.disc.filename = ClownCD_DuplicateString(file_path); /* It's okay for this to fail. */
	state.disc.file = stream != NULL ? ClownCD_FileOpenAlreadyOpen(stream, callbacks) : ClownCD_FileOpen(file_path, CLOWNCD_RB, callbacks);
	state.type = ClownCD_GetDiscType(&state.disc.file);

	state.disc.track.header_size = 0;
	state.disc.track.audio_decoder_needed = cc_false;
	state.disc.track.has_full_sized_sectors = cc_false;
	state.disc.track.starting_frame = 0;
	state.disc.track.current_frame = 0;
	state.disc.track.total_frames = 0;

	switch (state.type)
	{
		default:
			assert(cc_false);
			/* Fallthrough */
		case CLOWNCD_DISC_CUE:
			ClownCD_Disc_CueOpen(&state.disc);
			break;

		case CLOWNCD_DISC_RAW_2048:
		case CLOWNCD_DISC_RAW_2352:
			ClownCD_Disc_RawOpen(&state.disc);
			break;

		case CLOWNCD_DISC_CLOWNCD:
			ClownCD_Disc_ClownCDOpen(&state.disc);
			break;
	}

	return state;
}

void ClownCD_Close(ClownCD* const state)
{
	if (ClownCD_FileIsOpen(&state->disc.track.file))
	{
		if (state->disc.track.audio_decoder_needed)
			ClownCD_AudioClose(&state->disc.track.audio);

		ClownCD_FileClose(&state->disc.track.file);
	}

	if (ClownCD_FileIsOpen(&state->disc.file))
		ClownCD_FileClose(&state->disc.file);

	free(state->disc.filename);
}

static cc_bool ClownCD_SeekTrackIndexInternal(ClownCD* const state, const unsigned int track, const unsigned int index)
{
	if (track != state->disc.track.current_track || index != state->disc.track.current_index)
	{
		state->disc.track.current_track = track;
		state->disc.track.current_index = index;

		switch (state->type)
		{
			case CLOWNCD_DISC_CUE:
				if (!ClownCD_Disc_CueSeekTrackIndex(&state->disc, track, index))
					return cc_false;

				break;

			case CLOWNCD_DISC_RAW_2048:
			case CLOWNCD_DISC_RAW_2352:
				if (!ClownCD_Disc_RawSeekTrackIndex(&state->disc, track, index, state->type == CLOWNCD_DISC_RAW_2352))
					return cc_false;

				break;

			case CLOWNCD_DISC_CLOWNCD:
				if (!ClownCD_Disc_ClownCDSeekTrackIndex(&state->disc, track, index))
					return cc_false;

				break;
		}

		/* Force the frame to update. */
		state->disc.track.current_frame = -1;
	}

	return cc_true;
}

cc_bool ClownCD_SeekAudioFrame(ClownCD* const state, const size_t frame)
{
	if (frame >= state->disc.track.total_frames)
		return cc_false;

	if (frame != state->disc.track.current_frame)
	{
		state->disc.track.current_frame = frame;

		if (state->disc.track.audio_decoder_needed)
		{
			if (!ClownCD_AudioSeek(&state->disc.track.audio, state->disc.track.starting_frame + frame))
				return cc_false;
		}
		else
		{
			if (!ClownCD_SeekFrameInternal(state, state->disc.track.current_frame))
				return cc_false;
		}
	}

	return cc_true;
}

cc_bool ClownCD_SetState(ClownCD* const state, const unsigned int track, const unsigned int index, const size_t frame)
{
	if (!ClownCD_SeekTrackIndexInternal(state, track, index))
		return cc_false;

	if (!ClownCD_SeekAudioFrame(state, frame))
		return cc_false;

	return cc_true;
}

cc_bool ClownCD_BeginSectorStream(ClownCD* const state)
{
	if (!ClownCD_IsFrameValid(state))
		return cc_false;

	state->disc.track.current_frame += CLOWNCD_AUDIO_FRAMES_PER_SECTOR;

	/* Skip header. */
	if (state->disc.track.has_full_sized_sectors)
		if (ClownCD_FileSeek(&state->disc.track.file, CLOWNCD_SECTOR_HEADER_SIZE, CLOWNCD_SEEK_CUR) != 0)
			return cc_false;

	return cc_true;
}

size_t ClownCD_ReadSectorStream(ClownCD* const state, unsigned char* const buffer, const size_t total_bytes)
{
	return ClownCD_FileRead(buffer, 1, total_bytes, &state->disc.track.file);
}

cc_bool ClownCD_EndSectorStream(ClownCD* const state)
{
	/* Skip error-correction data. */
	if (state->disc.track.has_full_sized_sectors)
		if (ClownCD_FileSeek(&state->disc.track.file, CLOWNCD_SECTOR_RAW_SIZE - (CLOWNCD_SECTOR_HEADER_SIZE + CLOWNCD_SECTOR_DATA_SIZE), CLOWNCD_SEEK_CUR) != 0)
			return cc_false;

	return cc_true;
}

size_t ClownCD_ReadSector(ClownCD* const state, unsigned char* const buffer)
{
	size_t bytes_read;

	if (!ClownCD_BeginSectorStream(state))
		return 0;

	bytes_read = ClownCD_ReadSectorStream(state, buffer, CLOWNCD_SECTOR_DATA_SIZE);

	ClownCD_EndSectorStream(state);

	return bytes_read;
}

static size_t ClownCD_ReadFramesGetAudio(ClownCD* const state, short* const buffer, const size_t total_frames)
{
	const size_t frames_to_do = CC_MIN(state->disc.track.total_frames - state->disc.track.current_frame, total_frames);

	size_t frames_done;

	if (state->disc.track.audio_decoder_needed)
	{
		frames_done = ClownCD_AudioRead(&state->disc.track.audio, buffer, frames_to_do);
	}
	else
	{
		short *buffer_pointer = buffer;

		if (!state->disc.track.has_full_sized_sectors)
			return 0;

		for (frames_done = 0; frames_done < frames_to_do; ++frames_done)
		{
			*buffer_pointer++ = ClownCD_ReadS16LE(&state->disc.track.file);
			*buffer_pointer++ = ClownCD_ReadS16LE(&state->disc.track.file);

			if (state->disc.track.file.eof)
				break;
		}
	}

	state->disc.track.current_frame += frames_done;

	return frames_done;
}

static size_t ClownCD_ReadFramesGeneratePadding(ClownCD* const state, short* const buffer, const size_t total_frames)
{
	const size_t occupied_frames_in_sector = state->disc.track.current_frame % CLOWNCD_AUDIO_FRAMES_PER_SECTOR;
	const size_t empty_frames_in_sector = occupied_frames_in_sector == 0 ? 0 : CLOWNCD_AUDIO_FRAMES_PER_SECTOR - occupied_frames_in_sector;

	const size_t frames_to_do = CC_MIN(empty_frames_in_sector, total_frames);

	memset(buffer, 0, frames_to_do * CLOWNCD_AUDIO_FRAME_SIZE);

	return frames_to_do;
}

size_t ClownCD_ReadFrames(ClownCD* const state, short* const buffer, const size_t total_frames)
{
	const size_t audio_frames_done = ClownCD_ReadFramesGetAudio(state, buffer, total_frames);
	const size_t padding_frames_done = ClownCD_ReadFramesGeneratePadding(state, buffer + audio_frames_done * CLOWNCD_AUDIO_CHANNELS, total_frames - audio_frames_done);

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
