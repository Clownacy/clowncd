#ifndef CLOWNCD_H
#define CLOWNCD_H

#include <stdio.h>

#include "clowncommon/clowncommon.h"

typedef struct ClownCD
{
	FILE *file;
} ClownCD;

cc_bool ClownCD_OpenFromFile(ClownCD *disc, const char *file_path);
void ClownCD_Close(ClownCD *disc);

void ClownCD_ReadSectorRaw(ClownCD *disc, unsigned long sector_index, unsigned char *buffer);
void ClownCD_ReadSectorData(ClownCD *disc, unsigned long sector_index, unsigned char *buffer);
unsigned long ClownCD_CalculateSectorCRC(const unsigned char *buffer);
cc_bool ClownCD_ValidateSectorCRC(const unsigned char *buffer);

#endif /* CLOWNCD_H */
