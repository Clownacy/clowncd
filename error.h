#ifndef CLOWNCD_ERROR_H
#define CLOWNCD_ERROR_H

typedef void (*ClownCD_ErrorCallback)(void *user_data, const char *message);

#ifdef __cplusplus
extern "C" {
#endif

void ClownCD_SetErrorCallback(ClownCD_ErrorCallback callback, const void *user_data);
void ClownCD_LogError(const char *message);

#ifdef __cplusplus
}
#endif

#endif /* CLOWNCD_ERROR_H */
