#ifndef CLOWNCD_ERROR_H
#define CLOWNCD_ERROR_H

typedef (*ClownCD_ErrorCallback)(const char *message);

void ClownCD_SetErrorCallback(ClownCD_ErrorCallback callback);
void ClownCD_LogError(const char *message);

#endif /* CLOWNCD_ERROR_H */
