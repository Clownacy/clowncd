#include "vorbis.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#define STB_VORBIS_NO_STDIO
#define STB_VORBIS_NO_PUSHDATA_API
#include "libraries/stb_vorbis.c"

static size_t ClownCD_VorbisFileSize(ClownCD_File* const file)
{
	const long position = ClownCD_FileTell(file);

	long size;

	if (ClownCD_FileSeek(file, 0, CLOWNCD_SEEK_END) != 0)
		return 0;

	size = ClownCD_FileTell(file);

	if (ClownCD_FileSeek(file, position, CLOWNCD_SEEK_SET) != 0)
		return 0;

	if (size < 0)
		return 0;

	return size;
}

static cc_bool ClownCD_VorbisFileToMemory(ClownCD_File* const file, void** const out_buffer, size_t* const out_size)
{
	if (out_buffer != NULL && out_size != NULL)
	{
		const size_t size = ClownCD_VorbisFileSize(file);
		void* const buffer = malloc(size);

		if (buffer != NULL)
		{
			if (ClownCD_FileSeek(file, 0, CLOWNCD_SEEK_SET) == 0)
			{
				if (ClownCD_FileRead(buffer, size, 1, file) == 1)
				{
					*out_buffer = buffer;
					*out_size = size;

					return cc_true;
				}
			}

			free(buffer);
		}
	}

	return cc_false;
}

cc_bool ClownCD_VorbisOpen(ClownCD_Vorbis* const vorbis, ClownCD_File* const file)
{
	size_t size;

	if (ClownCD_VorbisFileToMemory(file, &vorbis->file_buffer, &size))
	{
		vorbis->instance = stb_vorbis_open_memory((const unsigned char*)vorbis->file_buffer, size, NULL, NULL);

		if (vorbis->instance != NULL)
		{
			const stb_vorbis_info vorbis_info = stb_vorbis_get_info(vorbis->instance);

			if (vorbis_info.sample_rate == 44100 && vorbis_info.channels == 2)
				return cc_true;

			stb_vorbis_close(vorbis->instance);
		}

		free(vorbis->file_buffer);
	}

	return cc_false;
}

void ClownCD_VorbisClose(ClownCD_Vorbis* const vorbis)
{
	stb_vorbis_close(vorbis->instance);
	free(vorbis->file_buffer);
}

cc_bool ClownCD_VorbisSeek(ClownCD_Vorbis* const vorbis, const size_t frame)
{
	return stb_vorbis_seek_frame(vorbis->instance, frame) != 0;
}

size_t ClownCD_VorbisRead(ClownCD_Vorbis* const vorbis, short* const buffer, const size_t total_frames)
{
	return stb_vorbis_get_samples_short_interleaved(vorbis->instance, vorbis->instance->channels, buffer, total_frames * vorbis->instance->channels);
}
