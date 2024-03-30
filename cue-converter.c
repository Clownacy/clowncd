#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cue.h"
#include "file-io.h"

typedef struct State
{
	ClownCD_File *cue_file, *header_file;
	const char *cue_filename;
	char *track_filename;
} State;

static void* FileOpenCallback(const char* const filename, const int mode)
{
	static const char* const to_standard[] = {"rb", "wb"};

	return fopen(filename, to_standard[mode]);
}

static int FileCloseCallback(void* const stream)
{
	return fclose((FILE*)stream);
}

static size_t FileReadCallback(void* const buffer, const size_t size, const size_t count, void* const stream)
{
	return fread(buffer, size, count, (FILE*)stream);
}

static size_t FileWriteCallback(const void* const buffer, const size_t size, const size_t count, void* const stream)
{
	return fwrite(buffer, size, count, (FILE*)stream);
}

static long FileTellCallback(void* const stream)
{
	return ftell((FILE*)stream);
}

static int FileSeekCallback(void* const stream, const long position, const int origin)
{
	static const int to_standard[] = {SEEK_SET, SEEK_CUR, SEEK_END};

	return fseek((FILE*)stream, position, to_standard[origin]);
}

static ClownCD_File FileOpen(const char* const filename, const char* const mode)
{
	static const ClownCD_FileCallbacks file_callbacks = {FileOpenCallback, FileCloseCallback, FileReadCallback, FileWriteCallback, FileTellCallback, FileSeekCallback};

	ClownCD_File file;

	file.functions = &file_callbacks;
	file.stream = fopen(filename, mode);

	return file;
}

static int FileClose(ClownCD_File* const file)
{
	return fclose((FILE*)file->stream);
}

static void GetTrackIndexFrame_Callback(void* const user_data, const char* const filename, const Cue_FileType file_type, const unsigned int track, const Cue_TrackType track_type, const unsigned int index, const unsigned long frame)
{
	unsigned long* const frame_pointer = (unsigned long*)user_data;

	(void)filename;
	(void)file_type;
	(void)track;
	(void)track_type;
	(void)index;

	*frame_pointer = frame;
}

static unsigned long GetTrackIndexFrame(ClownCD_File* const file, const unsigned int track, const unsigned int index)
{
	unsigned long frame = 0xFFFFFFFF;
	Cue_GetTrackIndexInfo(file, track, index, GetTrackIndexFrame_Callback, &frame);
	return frame;
}

static char* GetLastPathSeparator(const char* const file_path)
{
	char* const forward_slash = strrchr(file_path, '/');
#ifdef _WIN32
	char* const back_slash = strrchr(file_path, '\\');

	if (forward_slash == NULL)
		return back_slash;
	else if (back_slash == NULL)
		return forward_slash;
	else
		return CC_MIN(forward_slash, back_slash);
#else
	return forward_slash;
#endif
}

static size_t GetIndexOfFilenameInPath(const char* const filename)
{
	const char* const separator = GetLastPathSeparator(filename);

	if (separator == NULL)
		return 0;

	return separator - filename + 1;
}

static char* GetFullFilePath(const char* const directory, const char* const filename)
{
	const size_t directory_length = GetIndexOfFilenameInPath(directory);
	const size_t filename_length = strlen(filename);
	char* const full_path = (char*)malloc(directory_length + filename_length + 1);

	if (full_path == NULL)
		return NULL;

	memcpy(full_path, directory, directory_length);
	memcpy(full_path + directory_length, filename, filename_length);
	full_path[directory_length + filename_length] = '\0';

	return full_path;
}

static size_t GetFileSize(const char* const filename)
{
	FILE* const file = fopen(filename, "rb");

	size_t file_size = -1;

	if (file == NULL)
	{
		fprintf(stderr, "Could not open file '%s'.\n", filename);
	}
	else
	{
		if (fseek(file, 0, SEEK_END) == 0)
			file_size = ftell(file);

		fclose(file);
	}

	return file_size;
}

static unsigned long GetTrackEndingFrame(const State* const state, const char* const track_filename, const unsigned int track)
{
	unsigned long ending_frame = GetTrackIndexFrame(state->cue_file, track + 1, 0);

	if (ending_frame == 0xFFFFFFFF)
	{
		/* If the pregap index is missing, then try the main index instead. */
		ending_frame = GetTrackIndexFrame(state->cue_file, track + 1, 1);

		if (ending_frame == 0xFFFFFFFF)
		{
			char* const full_path = GetFullFilePath(state->cue_filename, track_filename);

			if (full_path == NULL)
			{
				fputs("Could not allocate memory for full file path.\n", stderr);
			}
			else
			{
				const size_t track_file_size = GetFileSize(full_path);

				if (track_file_size != (size_t)-1)
				{
					if (track_file_size % 2352 != 0)
						fputs("Track file size is not a multiple of 2352.\n", stderr);

					ending_frame = track_file_size / 2352;
				}

				free(full_path);
			}
		}
	}

	return ending_frame;
}

static char* DuplicateString(const char* const string)
{
	const size_t length = strlen(string) + 1;
	char* const buffer = (char*)malloc(length);

	if (buffer != NULL)
		memcpy(buffer, string, length);

	return buffer;
}

static void Callback(void* const user_data, const char* const filename, const Cue_FileType file_type, const unsigned int track, const Cue_TrackType track_type, const unsigned int index, const unsigned long frame)
{
	State* const state = (State*)user_data;
	const unsigned long ending_frame = GetTrackEndingFrame(state, filename, track);

	unsigned int type;

	(void)index;

	if (state->track_filename == NULL)
	{
		/* TODO: Hash the filename instead of copy it. */
		state->track_filename = DuplicateString(filename);

		if (state->track_filename == NULL)
			fputs("Could not allocate memory to duplicate track filename.\n", stderr);
	}
	else
	{
		if (strcmp(state->track_filename, filename) != 0)
			fputs("All tracks must use the same file.\n", stderr);
	}

	if (file_type != CUE_FILE_TYPE_BINARY)
		fputs("Only FILE type BINARY is supported.", stderr);

	switch (track_type)
	{
		case CUE_TRACK_TYPE_MODE1_2048:
			fputs("MODE1/2048 tracks are not supported: use MODE1/2352 instead.", stderr);
			/* Fallthrough */
		case CUE_TRACK_TYPE_MODE1_2352:
			type = 0;
			break;

		case CUE_TRACK_TYPE_AUDIO:
			type = 1;
			break;

		default:
			fputs("Unknown track type encountered.\n", stderr);
			break;
	}

	Write16BE(state->header_file, type);
	Write32BE(state->header_file, frame);
	Write32BE(state->header_file, ending_frame - frame);
}

int main(const int argc, char** const argv)
{
	if (argc < 3)
	{
		fprintf(stderr, "Usage: %s input-filename output-filename\n", argv[0]);
	}
	else
	{
		const char* const cue_filename = argv[1];
		ClownCD_File cue_file = FileOpen(cue_filename, "r");

		if (cue_file.stream == NULL)
		{
			fputs("Could not open input file.\n", stderr);
		}
		else
		{
			ClownCD_File header_file = FileOpen(argv[2], "wb");

			if (header_file.stream == NULL)
			{
				fputs("Could not open output file.\n", stderr);
			}
			else
			{
				static const char identifier[8] = {'c', 'l', 'o', 'w', 'n', 'c', 'd', '\0'};
				State state;
				unsigned int i;

				state.cue_file = &cue_file;
				state.header_file = &header_file;
				state.cue_filename = cue_filename;
				state.track_filename = NULL;

				ClownCD_FileWrite(identifier, sizeof(identifier), 1, &header_file); /* Identifier. */
				Write16BE(&header_file, 0); /* Version. */
				Write16BE(&header_file, 0); /* Total tracks (will be filled-in later). */

				for (i = 0; ; ++i)
					if (!Cue_GetTrackIndexInfo(&cue_file, i + 1, 1, Callback, &state))
						break;

				ClownCD_FileSeek(&header_file, 8 + 2, CLOWNCD_SEEK_SET);
				Write16BE(&header_file, i); /* Total tracks. */

				FileClose(&header_file);
			}

			FileClose(&cue_file);
		}
	}

	return EXIT_SUCCESS;
}
