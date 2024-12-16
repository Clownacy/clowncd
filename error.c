#include "error.h"

#include <stddef.h>

static ClownCD_ErrorCallback callback;

void ClownCD_SetErrorCallback(const ClownCD_ErrorCallback callback_parameter)
{
	callback = callback_parameter;
}

void ClownCD_LogError(const char* const message)
{
	if (callback != NULL)
		callback(message);
}
