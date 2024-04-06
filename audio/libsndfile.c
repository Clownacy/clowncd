#include "libsndfile.h"

#include <assert.h>

static sf_count_t ClownCD_libSndFileReadCallback(void* const output, const sf_count_t count, void* const user_data)
{
	ClownCD_File* const file = (ClownCD_File*)user_data;

	return ClownCD_FileRead(output, 1, count, file);
}

static sf_count_t ClownCD_libSndFileSeekCallback(const sf_count_t offset, const int origin, void* const user_data)
{
	ClownCD_File* const file = (ClownCD_File*)user_data;

	ClownCD_FileOrigin ccd_origin;

	switch (origin)
	{
		case SF_SEEK_SET:
			ccd_origin = CLOWNCD_SEEK_SET;
			break;

		case SF_SEEK_CUR:
			ccd_origin = CLOWNCD_SEEK_CUR;
			break;

		case SF_SEEK_END:
			ccd_origin = CLOWNCD_SEEK_END;
			break;

		default:
			return -1;
	}

	if (ClownCD_FileSeek(file, offset, ccd_origin) != 0)
		return -1;

	return ClownCD_FileTell(file);
}

static sf_count_t ClownCD_libSndFileTellCallback(void* const user_data)
{
	ClownCD_File* const file = (ClownCD_File*)user_data;

	return ClownCD_FileTell(file);
}

static sf_count_t ClownCD_libSndFileSizeCallback(void* const user_data)
{
	ClownCD_File* const file = (ClownCD_File*)user_data;

	return ClownCD_FileSize(file);
}

cc_bool ClownCD_libSndFileOpen(ClownCD_libSndFile* const track, ClownCD_File* const file)
{
	SF_VIRTUAL_IO callbacks = {
		ClownCD_libSndFileSizeCallback,
		ClownCD_libSndFileSeekCallback,
		ClownCD_libSndFileReadCallback,
		NULL,
		ClownCD_libSndFileTellCallback
	};
	SF_INFO info = {0};

	if (ClownCD_FileSeek(file, 0, CLOWNCD_SEEK_SET) != 0)
		return cc_false;

	track->libsndfile = sf_open_virtual(&callbacks, SFM_READ, &info, file);

	if (track->libsndfile == NULL)
	{
		const char* const error_message = sf_strerror(NULL);
		fputs(error_message, stderr);
	}
	else
	{
		/* Prevent popping caused by the float->integer conversion. */
		sf_command(track->libsndfile, SFC_SET_SCALE_FLOAT_INT_READ, NULL, SF_TRUE);

		/* Verify that the audio is in a supported format. */
		if (info.channels == 2 && info.samplerate == 44100)
			return cc_true;

		sf_close(track->libsndfile);
	}

	return cc_false;
}

void ClownCD_libSndFileClose(ClownCD_libSndFile* const track)
{
	sf_close(track->libsndfile);
}

cc_bool ClownCD_libSndFileSeek(ClownCD_libSndFile* const track, const size_t frame)
{
	return sf_seek(track->libsndfile, frame, SF_SEEK_SET) != -1;
}

size_t ClownCD_libSndFileRead(ClownCD_libSndFile* const track, short* const buffer, const size_t total_frames)
{
	return sf_readf_short(track->libsndfile, buffer, total_frames);
}
