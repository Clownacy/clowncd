#include "cue.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"

typedef enum ClownCD_CueCommandType
{
	CLOWNCD_CUE_COMMAND_INVALID,
	CLOWNCD_CUE_COMMAND_CATALOG,
	CLOWNCD_CUE_COMMAND_CDTEXTFILE,
	CLOWNCD_CUE_COMMAND_FILE,
	CLOWNCD_CUE_COMMAND_FLAGS,
	CLOWNCD_CUE_COMMAND_INDEX,
	CLOWNCD_CUE_COMMAND_ISRC,
	CLOWNCD_CUE_COMMAND_PERFORMER,
	CLOWNCD_CUE_COMMAND_POSTGAP,
	CLOWNCD_CUE_COMMAND_PREGAP,
	CLOWNCD_CUE_COMMAND_REM,
	CLOWNCD_CUE_COMMAND_SONGWRITER,
	CLOWNCD_CUE_COMMAND_TITLE,
	CLOWNCD_CUE_COMMAND_TRACK
} ClownCD_CueCommandType;

static ClownCD_CueCommandType ClownCD_CueCommandTypeFromString(const char* const string)
{
	static const struct
	{
		const char *string;
		ClownCD_CueCommandType command_type;
	} commands[] = {
		{"CATALOG"   , CLOWNCD_CUE_COMMAND_CATALOG},
		{"CDTEXTFILE", CLOWNCD_CUE_COMMAND_CDTEXTFILE},
		{"FILE"      , CLOWNCD_CUE_COMMAND_FILE},
		{"FLAGS"     , CLOWNCD_CUE_COMMAND_FLAGS},
		{"INDEX"     , CLOWNCD_CUE_COMMAND_INDEX},
		{"ISRC"      , CLOWNCD_CUE_COMMAND_ISRC},
		{"PERFORMER" , CLOWNCD_CUE_COMMAND_PERFORMER},
		{"POSTGAP"   , CLOWNCD_CUE_COMMAND_POSTGAP},
		{"PREGAP"    , CLOWNCD_CUE_COMMAND_PREGAP},
		{"REM"       , CLOWNCD_CUE_COMMAND_REM},
		{"SONGWRITER", CLOWNCD_CUE_COMMAND_SONGWRITER},
		{"TITLE"     , CLOWNCD_CUE_COMMAND_TITLE},
		{"TRACK"     , CLOWNCD_CUE_COMMAND_TRACK},
	};

	size_t i;

	for (i = 0; i < CC_COUNT_OF(commands); ++i)
		if (strcmp(string, commands[i].string) == 0)
			return commands[i].command_type;

	return CLOWNCD_CUE_COMMAND_INVALID;
}

static ClownCD_CueFileType ClownCD_CueFileTypeFromString(const char* const string)
{
	if (strcmp(string, "BINARY") == 0)
		return CLOWNCD_CUE_FILE_BINARY;
	else if (strcmp(string, "WAVE") == 0)
		return CLOWNCD_CUE_FILE_WAVE;
	else if (strcmp(string, "MP3") == 0)
		return CLOWNCD_CUE_FILE_MP3;
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
		const unsigned long character = ClownCD_ReadU8(file);

		if (file->eof)
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

cc_bool ClownCD_CueParse(ClownCD_File* const file, const ClownCD_CueCallback callback, const void* const user_data)
{
	const long starting_file_position = ClownCD_FileTell(file);

	char *file_name = NULL;
	ClownCD_CueFileType file_type = CLOWNCD_CUE_FILE_INVALID;
	unsigned int track = 0xFFFF;
	ClownCD_CueTrackType track_type = CLOWNCD_CUE_TRACK_INVALID;
	cc_bool valid = cc_true;

	ClownCD_FileSeek(file, 0, CLOWNCD_SEEK_SET);

	while (valid)
	{
		char* const line = ClownCD_CueReadLine(file);

		char command_string[10 + 1];

		if (line == NULL)
			break;

		if (sscanf(line, "%10s", command_string) == 1)
		{
			switch (ClownCD_CueCommandTypeFromString(command_string))
			{
				case CLOWNCD_CUE_COMMAND_FILE:
				{
					char first_character;

					if (sscanf(line, " FILE %c", &first_character) == 1)
					{
						int file_name_start = 0, file_name_end = 0;
						char file_type_string[6 + 1];

						if (first_character == '"')
						{
							if (sscanf(line, " FILE \"%n%*[^\"]%n\" %6s", &file_name_start, &file_name_end, file_type_string) != 1)
								file_name_start = 0;
						}
						else
						{
							if (sscanf(line, " FILE %n%*s%n %6s", &file_name_start, &file_name_end, file_type_string) != 1)
								file_name_start = 0;
						}

						if (file_name_start != 0 && file_name_end > file_name_start)
						{
							const int file_name_length = file_name_end - file_name_start;

							free(file_name);
							file_name = (char*)malloc(file_name_length + 1);

							if (file_name == NULL)
							{
								ClownCD_LogError("Could not allocate memory for filename.\n");
							}
							else
							{
								memcpy(file_name, line + file_name_start, file_name_length);
								file_name[file_name_length] = '\0';

								file_type = ClownCD_CueFileTypeFromString(file_type_string);
								break;
							}
						}
					}

					free(file_name);
					file_name = NULL;

					valid = cc_false;

					break;
				}

				case CLOWNCD_CUE_COMMAND_TRACK:
				{
					char track_type_string[10 + 1];

					if (sscanf(line, " TRACK %u %10s", &track, track_type_string) < 2)
						ClownCD_LogError("Could not read TRACK parameters.\n");
					else
					{
						track_type = ClownCD_CueTrackTypeFromString(track_type_string);
						break;
					}

					valid = cc_false;
					break;
				}

				case CLOWNCD_CUE_COMMAND_INDEX:
				{
					unsigned int index, minute, second, sector;

					if (sscanf(line, " INDEX %u %u:%u:%u", &index, &minute, &second, &sector) < 4)
						ClownCD_LogError("Could not read INDEX parameters.\n");
					else if (file_name == NULL)
						ClownCD_LogError("INDEX encountered with no filename specified.\n");
					else if (file_type == CLOWNCD_CUE_FILE_INVALID)
						ClownCD_LogError("INDEX encountered with no file type specified.\n");
					else if (track == 0xFFFF)
						ClownCD_LogError("INDEX encountered with no track specified.\n");
					else if (track_type == CLOWNCD_CUE_TRACK_INVALID)
						ClownCD_LogError("INDEX encountered with no track type specified.\n");
					else
					{
						if (callback != NULL)
							callback((void*)user_data, file_name, file_type, track, track_type, index, ((unsigned long)minute * 60 + second) * 75 + sector);

						break;
					}

					valid = cc_false;
					break;
				}

				case CLOWNCD_CUE_COMMAND_CATALOG:
				case CLOWNCD_CUE_COMMAND_CDTEXTFILE:
				case CLOWNCD_CUE_COMMAND_FLAGS:
				case CLOWNCD_CUE_COMMAND_ISRC:
				case CLOWNCD_CUE_COMMAND_PERFORMER:
				case CLOWNCD_CUE_COMMAND_POSTGAP:
				case CLOWNCD_CUE_COMMAND_PREGAP:
				case CLOWNCD_CUE_COMMAND_REM:
				case CLOWNCD_CUE_COMMAND_SONGWRITER:
				case CLOWNCD_CUE_COMMAND_TITLE:
					/* We do not care about these. */
					break;

				default:
				case CLOWNCD_CUE_COMMAND_INVALID:
					valid = cc_false;
					break;
			}
		}

		free(line);
	}

	free(file_name);
	ClownCD_FileSeek(file, starting_file_position, CLOWNCD_SEEK_SET);

	return valid;
}

typedef struct ClownCD_CueGetTrackIndexInfo_State
{
	unsigned int track, index;
	ClownCD_CueCallback callback;
	void *user_data;
	cc_bool found;
} ClownCD_CueGetTrackIndexInfo_State;

static void ClownCD_CueGetTrackIndexInfo_Callback(void* const user_data, const char* const filename, const ClownCD_CueFileType file_type, const unsigned int track, const ClownCD_CueTrackType track_type, const unsigned int index, const unsigned long sector)
{
	ClownCD_CueGetTrackIndexInfo_State* const state = (ClownCD_CueGetTrackIndexInfo_State*)user_data;

	if (state->track == track && state->index == index)
	{
		if (state->callback != NULL)
			state->callback(state->user_data, filename, file_type, track, track_type, index, sector);

		state->found = cc_true;

		/* TODO: Have this return a value to stop the parsing. */
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

typedef struct ClownCD_CueGetTrackEndingSector_State
{
	const char *track_index_filename;
	unsigned int track, index;
	unsigned long starting_sector, ending_sector;
} ClownCD_CueGetTrackEndingSector_State;

static void ClownCD_CueGetTrackEndingSector_Callback(void* const user_data, const char* const filename, const ClownCD_CueFileType file_type, const unsigned int track, const ClownCD_CueTrackType track_type, const unsigned int index, const unsigned long sector)
{
	ClownCD_CueGetTrackEndingSector_State* const state = (ClownCD_CueGetTrackEndingSector_State*)user_data;

	(void)file_type;
	(void)track_type;

	if (strcmp(filename, state->track_index_filename) == 0 && (track != state->track || index != state->index) && sector > state->starting_sector && sector < state->ending_sector)
		state->ending_sector = sector;
}

unsigned long ClownCD_CueGetTrackIndexEndingSector(ClownCD_File* const file, const char* const track_index_filename, const unsigned int track, const unsigned int index, const unsigned long starting_sector)
{
	ClownCD_CueGetTrackEndingSector_State state;

	state.track_index_filename = track_index_filename;
	state.track = track;
	state.index = index;
	state.starting_sector = starting_sector;
	state.ending_sector = 0xFFFFFFFF;

	ClownCD_CueParse(file, ClownCD_CueGetTrackEndingSector_Callback, &state);

	return state.ending_sector;
}
