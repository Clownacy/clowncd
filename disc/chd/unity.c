/* Disable libchdr features that we do not need. */
#define WANT_RAW_DATA_SECTOR 0
#define WANT_SUBCODE 0
#define VERIFY_BLOCK_CRC 0

#include "libchdr/unity.c"

#include "streams/chd_stream.c"
