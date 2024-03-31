#include "utilities.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "clowncommon/clowncommon.h"

static char* ClownCD_GetLastPathSeparator(const char* const file_path)
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

static size_t ClownCD_GetIndexOfFilenameInPath(const char* const filename)
{
	const char* const separator = ClownCD_GetLastPathSeparator(filename);

	if (separator == NULL)
		return 0;

	return separator - filename + 1;
}

char* ClownCD_GetFullFilePath(const char* const directory, const char* const filename)
{
	if (directory == NULL)
	{
		return ClownCD_DuplicateString(filename);
	}
	else
	{
		const size_t directory_length = ClownCD_GetIndexOfFilenameInPath(directory);
		const size_t filename_length = strlen(filename);
		char* const full_path = (char*)malloc(directory_length + filename_length + 1);

		if (full_path == NULL)
			return NULL;

		memcpy(full_path, directory, directory_length);
		memcpy(full_path + directory_length, filename, filename_length);
		full_path[directory_length + filename_length] = '\0';

		return full_path;
	}
}

char* ClownCD_DuplicateString(const char* const string)
{
	if (string == NULL)
	{
		return NULL;
	}
	else
	{
		const size_t length = strlen(string) + 1;
		char* const buffer = (char*)malloc(length);

		if (buffer != NULL)
			memcpy(buffer, string, length);

		return buffer;
	}
}
