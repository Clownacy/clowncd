#include "error.h"

#include <stddef.h>

static ClownCD_ErrorCallback clowncd_callback;
static void* clowncd_callback_user_data;

void ClownCD_SetErrorCallback(const ClownCD_ErrorCallback callback, const void* const user_data)
{
	clowncd_callback = callback;
	clowncd_callback_user_data = (void*)user_data;
}

void ClownCD_LogError(const char* const message)
{
	if (clowncd_callback != NULL)
		clowncd_callback(clowncd_callback_user_data, message);
}
