#include "cue.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum Cue_CommandType
{
	CUE_COMMAND_TYPE_INVALID,
	CUE_COMMAND_TYPE_FILE,
	CUE_COMMAND_TYPE_TRACK,
	CUE_COMMAND_TYPE_INDEX,
	CUE_COMMAND_TYPE_PREGAP
} Cue_CommandType;

static Cue_CommandType Cue_CommandTypeFromString(const char* const string)
{
	if (strcmp(string, "FILE") == 0)
		return CUE_COMMAND_TYPE_FILE;
	else if (strcmp(string, "TRACK") == 0)
		return CUE_COMMAND_TYPE_TRACK;
	else if (strcmp(string, "INDEX") == 0)
		return CUE_COMMAND_TYPE_INDEX;
	else if (strcmp(string, "PREGAP") == 0)
		return CUE_COMMAND_TYPE_PREGAP;
	else
		return CUE_COMMAND_TYPE_INVALID;	
}

static Cue_FileType Cue_FileTypeFromString(const char* const string)
{
	if (strcmp(string, "BINARY") == 0)
		return CUE_FILE_TYPE_BINARY;
	else
		return CUE_FILE_TYPE_INVALID;
}

static Cue_TrackType Cue_TrackTypeFromString(const char* const string)
{
	if (strcmp(string, "MODE1/2048") == 0)
		return CUE_TRACK_TYPE_MODE1_2048;
	else if (strcmp(string, "MODE1/2352") == 0)
		return CUE_TRACK_TYPE_MODE1_2352;
	else if (strcmp(string, "AUDIO") == 0)
		return CUE_TRACK_TYPE_AUDIO;
	else
		return CUE_TRACK_TYPE_INVALID;
}

static size_t Cue_GetLineLength(ClownCD_File* const file)
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

static char* Cue_ReadLine(ClownCD_File* const file)
{
	const size_t line_length = Cue_GetLineLength(file);
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

void Cue_Parse(ClownCD_File* const file, const Cue_Callback callback, const void* const user_data)
{
	const long starting_file_position = ClownCD_FileTell(file);

	char *file_name = NULL;
	Cue_FileType file_type = CUE_FILE_TYPE_INVALID;
	unsigned int track = 0xFFFF;
	Cue_TrackType track_type = CUE_TRACK_TYPE_INVALID;

	ClownCD_FileSeek(file, 0, CLOWNCD_SEEK_SET);

	for (;;)
	{
		char* const line = Cue_ReadLine(file);
		char *line_pointer = line;

		int advance;
		char command_string[6 + 1];

		if (line == NULL)
			break;

		if (sscanf(line_pointer, "%6s%n", command_string, &advance) == 1)
		{
			line_pointer += advance;

			switch (Cue_CommandTypeFromString(command_string))
			{
				case CUE_COMMAND_TYPE_FILE:
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
							file_type = Cue_FileTypeFromString(file_type_string);
						line_pointer += advance;
					}

					break;
				}

				case CUE_COMMAND_TYPE_TRACK:
				{
					char track_type_string[10 + 1];

					if (sscanf(line_pointer, "%u %10s%n", &track, track_type_string, &advance) < 2)
						fputs("Could not read TRACK parameters.\n", stderr);
					else
						track_type = Cue_TrackTypeFromString(track_type_string);
					line_pointer += advance;

					break;
				}

				case CUE_COMMAND_TYPE_INDEX:
				{
					unsigned int index, minute, second, frame;

					if (sscanf(line_pointer, "%u %u:%u:%u%n", &index, &minute, &second, &frame, &advance) < 4)
						fputs("Could not read INDEX parameters.\n", stderr);
					else if (file_name == NULL)
						fputs("INDEX encountered with no filename specified.\n", stderr);
					else if (file_type == CUE_FILE_TYPE_INVALID)
						fputs("INDEX encountered with no file type specified.\n", stderr);
					else if (track == 0xFFFF)
						fputs("INDEX encountered with no track specified.\n", stderr);
					else if (track_type == CUE_TRACK_TYPE_INVALID)
						fputs("INDEX encountered with no track type specified.\n", stderr);
					else
						callback((void*)user_data, file_name, file_type, track, track_type, index, ((unsigned long)minute * 60 + second) * 75 + frame);
					line_pointer += advance;

					break;
				}

				case CUE_COMMAND_TYPE_PREGAP:
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

typedef struct Cue_GetTrackIndexInfo_State
{
	unsigned int track, index;
	Cue_Callback callback;
	void *user_data;
	cc_bool found;
} Cue_GetTrackIndexInfo_State;

static void Cue_GetTrackIndexInfo_Callback(void* const user_data, const char* const filename, const Cue_FileType file_type, const unsigned int track, const Cue_TrackType track_type, const unsigned int index, const unsigned long frame)
{
	Cue_GetTrackIndexInfo_State* const state = (Cue_GetTrackIndexInfo_State*)user_data;

	if (state->track == track && state->index == index)
	{
		state->callback(state->user_data, filename, file_type, track, track_type, index, frame);
		state->found = cc_true;
	}
}

cc_bool Cue_GetTrackIndexInfo(ClownCD_File* const file, const unsigned int track, const unsigned int index, const Cue_Callback callback, const void* const user_data)
{
	Cue_GetTrackIndexInfo_State state;

	state.track = track;
	state.index = index;
	state.callback = callback;
	state.user_data = (void*)user_data;
	state.found = cc_false;

	Cue_Parse(file, Cue_GetTrackIndexInfo_Callback, &state);

	return state.found;
}
