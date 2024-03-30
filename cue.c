#include "cue.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum ClownCD_CueCommandType
{
	CLOWNCD_CUE_COMMAND_INVALID,
	CLOWNCD_CUE_COMMAND_FILE,
	CLOWNCD_CUE_COMMAND_TRACK,
	CLOWNCD_CUE_COMMAND_INDEX,
	CLOWNCD_CUE_COMMAND_PREGAP
} ClownCD_CueCommandType;

static ClownCD_CueCommandType ClownCD_CueCommandTypeFromString(const char* const string)
{
	if (strcmp(string, "FILE") == 0)
		return CLOWNCD_CUE_COMMAND_FILE;
	else if (strcmp(string, "TRACK") == 0)
		return CLOWNCD_CUE_COMMAND_TRACK;
	else if (strcmp(string, "INDEX") == 0)
		return CLOWNCD_CUE_COMMAND_INDEX;
	else if (strcmp(string, "PREGAP") == 0)
		return CLOWNCD_CUE_COMMAND_PREGAP;
	else
		return CLOWNCD_CUE_COMMAND_INVALID;
}

static ClownCD_CueFileType ClownCD_CueFileTypeFromString(const char* const string)
{
	if (strcmp(string, "BINARY") == 0)
		return CLOWNCD_CUE_FILE_BINARY;
	else
		return CLOWNCD_CUE_FILE_INVALID;
}

static ClownCD_CueTrackType ClownCD_CueTrackTypeFromString(const char* const string)
{
	if (strcmp(string, "MODE1/2048") == 0)
		return CLOWNCD_CUE_TRACK_MODE1_2048;
	else if (strcmp(string, "MODE1/2352") == 0)
		return CLOWNCD_CUE_TRACK_MODE1_2352;
	else if (strcmp(string, "AUDIO") == 0)
		return CLOWNCD_CUE_TRACK_AUDIO;
	else
		return CLOWNCD_CUE_TRACK_INVALID;
}

static size_t ClownCD_CueGetLineLength(ClownCD_File* const file)
{
	const long line_file_position = ClownCD_FileTell(file);
	size_t line_length = 0;

	for (;;)
	{
		const unsigned long character = ClownCD_Read8(file);

		if (character == (unsigned long)CLOWNCD_EOF)
			break;

		++line_length;

		if (character == '\r' || character == '\n')
			break;
	}

	ClownCD_FileSeek(file, line_file_position, CLOWNCD_SEEK_SET);

	return line_length;
}

static char* ClownCD_CueReadLine(ClownCD_File* const file)
{
	const size_t line_length = ClownCD_CueGetLineLength(file);
	char *line = (char*)malloc(line_length + 1);

	if (line != NULL)
	{
		if (ClownCD_FileRead(line, line_length, 1, file) != 1)
		{
			free(line);
			line = NULL;
		}
		else
		{
			line[line_length] = '\0';
		}
	}

	return line;
}

void ClownCD_CueParse(ClownCD_File* const file, const ClownCD_CueCallback callback, const void* const user_data)
{
	const long starting_file_position = ClownCD_FileTell(file);

	char *file_name = NULL;
	ClownCD_CueFileType file_type = CLOWNCD_CUE_FILE_INVALID;
	unsigned int track = 0xFFFF;
	ClownCD_CueTrackType track_type = CLOWNCD_CUE_TRACK_INVALID;

	ClownCD_FileSeek(file, 0, CLOWNCD_SEEK_SET);

	for (;;)
	{
		char* const line = ClownCD_CueReadLine(file);
		char *line_pointer = line;

		int advance;
		char command_string[6 + 1];

		if (line == NULL)
			break;

		if (sscanf(line_pointer, "%6s%n", command_string, &advance) == 1)
		{
			line_pointer += advance;

			switch (ClownCD_CueCommandTypeFromString(command_string))
			{
				case CLOWNCD_CUE_COMMAND_FILE:
				{
					int file_name_length;

					sscanf(line_pointer, " \"%n", &advance);
					line_pointer += advance;
					sscanf(line_pointer, "%*[^\"]%n", &file_name_length);

					free(file_name);
					file_name = (char*)malloc(file_name_length + 1);

					if (file_name == NULL)
					{
						fputs("Could not allocate memory for filename.\n", stderr);
					}
					else
					{
						char file_type_string[6 + 1];

						if (sscanf(line_pointer, "%[^\"]\" %6s%n", file_name, file_type_string, &advance) < 2)
							fputs("Could not read FILE parameters.\n", stderr);
						else
							file_type = ClownCD_CueFileTypeFromString(file_type_string);
						line_pointer += advance;
					}

					break;
				}

				case CLOWNCD_CUE_COMMAND_TRACK:
				{
					char track_type_string[10 + 1];

					if (sscanf(line_pointer, "%u %10s%n", &track, track_type_string, &advance) < 2)
						fputs("Could not read TRACK parameters.\n", stderr);
					else
						track_type = ClownCD_CueTrackTypeFromString(track_type_string);
					line_pointer += advance;

					break;
				}

				case CLOWNCD_CUE_COMMAND_INDEX:
				{
					unsigned int index, minute, second, frame;

					if (sscanf(line_pointer, "%u %u:%u:%u%n", &index, &minute, &second, &frame, &advance) < 4)
						fputs("Could not read INDEX parameters.\n", stderr);
					else if (file_name == NULL)
						fputs("INDEX encountered with no filename specified.\n", stderr);
					else if (file_type == CLOWNCD_CUE_FILE_INVALID)
						fputs("INDEX encountered with no file type specified.\n", stderr);
					else if (track == 0xFFFF)
						fputs("INDEX encountered with no track specified.\n", stderr);
					else if (track_type == CLOWNCD_CUE_TRACK_INVALID)
						fputs("INDEX encountered with no track type specified.\n", stderr);
					else
						callback((void*)user_data, file_name, file_type, track, track_type, index, ((unsigned long)minute * 60 + second) * 75 + frame);
					line_pointer += advance;

					break;
				}

				case CLOWNCD_CUE_COMMAND_PREGAP:
					/* We do not care about this. */
					break;

				default:
					fprintf(stderr, "Unrecognised command '%s'.\n", command_string);
					break;
			}
		}

		free(line);
	}

	free(file_name);
	ClownCD_FileSeek(file, starting_file_position, CLOWNCD_SEEK_SET);
}

typedef struct ClownCD_CueGetTrackIndexInfo_State
{
	unsigned int track, index;
	ClownCD_CueCallback callback;
	void *user_data;
	cc_bool found;
} ClownCD_CueGetTrackIndexInfo_State;

static void ClownCD_CueGetTrackIndexInfo_Callback(void* const user_data, const char* const filename, const ClownCD_CueFileType file_type, const unsigned int track, const ClownCD_CueTrackType track_type, const unsigned int index, const unsigned long frame)
{
	ClownCD_CueGetTrackIndexInfo_State* const state = (ClownCD_CueGetTrackIndexInfo_State*)user_data;

	if (state->track == track && state->index == index)
	{
		state->callback(state->user_data, filename, file_type, track, track_type, index, frame);
		state->found = cc_true;
	}
}

cc_bool ClownCD_CueGetTrackIndexInfo(ClownCD_File* const file, const unsigned int track, const unsigned int index, const ClownCD_CueCallback callback, const void* const user_data)
{
	ClownCD_CueGetTrackIndexInfo_State state;

	state.track = track;
	state.index = index;
	state.callback = callback;
	state.user_data = (void*)user_data;
	state.found = cc_false;

	ClownCD_CueParse(file, ClownCD_CueGetTrackIndexInfo_Callback, &state);

	return state.found;
}
