#include "clowncd.h"

#define SECTOR_RAW_SIZE 2352
#define SECTOR_HEADER_SIZE 0x10
#define SECTOR_DATA_SIZE 0x800

#define CRC_POLYNOMIAL 0xD8018001

cc_bool ClownCD_OpenFromFile(ClownCD* const disc, const char* const file_path)
{
	disc->file = ClownCD_FileOpen(file_path, CLOWNCD_MODE_READ);

	return disc->file.stream != NULL;
}

void ClownCD_Close(ClownCD* const disc)
{
	ClownCD_FileClose(&disc->file);
}

void ClownCD_ReadSectorRaw(ClownCD* const disc, const unsigned long sector_index, unsigned char* const buffer)
{
	ClownCD_FileSeek(&disc->file, sector_index * SECTOR_RAW_SIZE, CLOWNCD_SEEK_SET);
	ClownCD_FileRead(buffer, SECTOR_RAW_SIZE, 1, &disc->file);
}

void ClownCD_ReadSectorData(ClownCD* const disc, const unsigned long sector_index, unsigned char* const buffer)
{
	ClownCD_FileSeek(&disc->file, sector_index * SECTOR_RAW_SIZE + SECTOR_HEADER_SIZE, CLOWNCD_SEEK_SET);
	ClownCD_FileRead(buffer, SECTOR_DATA_SIZE, 1, &disc->file);
}

unsigned long ClownCD_CalculateSectorCRC(const unsigned char* const buffer)
{
	unsigned long shift, i;

	shift = 0;

	for (i = 0; i < (SECTOR_HEADER_SIZE + SECTOR_DATA_SIZE) * 8; ++i)
	{
		const unsigned int bit = i % 8;
		const unsigned int byte = i / 8;

		const unsigned long popped_bit = shift & 1;

		shift >>= 1;

		shift |= (unsigned long)((buffer[byte] >> bit) & 1) << 31;

		if (popped_bit != 0)
			shift ^= CRC_POLYNOMIAL;
	}

	for (i = 0; i < 32; ++i)
	{
		const unsigned long popped_bit = shift & 1;

		shift >>= 1;

		if (popped_bit != 0)
			shift ^= CRC_POLYNOMIAL;
	}

	return shift;
}

cc_bool ClownCD_ValidateSectorCRC(const unsigned char* const buffer)
{
	const unsigned long old_crc = ClownCD_Read32LEMemory(&buffer[SECTOR_HEADER_SIZE + SECTOR_DATA_SIZE]);
	const unsigned long new_crc = ClownCD_CalculateSectorCRC(buffer);

	return new_crc == old_crc;
}
