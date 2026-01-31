#include "vorbis.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#define STB_VORBIS_NO_STDIO
#define STB_VORBIS_NO_PUSHDATA_API
#define STB_VORBIS_MAX_CHANNELS 2
#define STB_VORBIS_NO_COMMENTS

/* This file is modified to replace all instances of 'stb_vorbis' with 'clowncd_stb_vorbis',
   to prevent linker conflicts with software which also uses stb_vorbis, such as
   PlayStation Vita builds of RetroArch. */
#include "libraries/stb_vorbis.c"

/* Undefine these REALLY STUPID macros, to avoid name conflicts in unity builds. */
#undef L
#undef C
#undef R

#include "../utilities.h"

static cc_bool ClownCD_VorbisFileToMemory(ClownCD_File* const file, void** const out_buffer, size_t* const out_size)
{
	if (out_buffer != NULL && out_size != NULL)
	{
		const size_t size = ClownCD_FileSize(file);

		if (size != CLOWNCD_SIZE_INVALID)
		{
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
	}

	return cc_false;
}

cc_bool ClownCD_VorbisOpen(ClownCD_Vorbis* const vorbis, ClownCD_File* const file, ClownCD_AudioMetadata* const metadata)
{
	size_t size;

	if (ClownCD_VorbisFileToMemory(file, &vorbis->file_buffer, &size))
	{
		vorbis->instance = clowncd_stb_vorbis_open_memory((const unsigned char*)vorbis->file_buffer, size, NULL, NULL);

		if (vorbis->instance != NULL)
		{
			const clowncd_stb_vorbis_info vorbis_info = clowncd_stb_vorbis_get_info(vorbis->instance);

			metadata->sample_rate = vorbis_info.sample_rate;
			metadata->total_channels = vorbis_info.channels;

			return cc_true;
		}

		free(vorbis->file_buffer);
	}

	return cc_false;
}

void ClownCD_VorbisClose(ClownCD_Vorbis* const vorbis)
{
	clowncd_stb_vorbis_close(vorbis->instance);
	free(vorbis->file_buffer);
}

cc_bool ClownCD_VorbisSeek(ClownCD_Vorbis* const vorbis, const size_t frame)
{
	return clowncd_stb_vorbis_seek_frame(vorbis->instance, frame) != 0;
}

size_t ClownCD_VorbisRead(ClownCD_Vorbis* const vorbis, short* const buffer, const size_t total_frames)
{
	return clowncd_stb_vorbis_get_samples_short_interleaved(vorbis->instance, vorbis->instance->channels, buffer, total_frames * vorbis->instance->channels);
}
