/* Namespace some symbols to avoid linker errors in static libretro builds. */
#define ClownCD_CHD_LzmaDec_InitDicAndState LzmaDec_InitDicAndState
#define ClownCD_CHD_LzmaDec_Init LzmaDec_Init
#define ClownCD_CHD_LzmaDec_DecodeToDic LzmaDec_DecodeToDic
#define ClownCD_CHD_LzmaDec_DecodeToBuf LzmaDec_DecodeToBuf
#define ClownCD_CHD_LzmaDec_FreeProbs LzmaDec_FreeProbs
#define ClownCD_CHD_LzmaDec_Free LzmaDec_Free
#define ClownCD_CHD_LzmaProps_Decode LzmaProps_Decode
#define ClownCD_CHD_LzmaDec_AllocateProbs LzmaDec_AllocateProbs
#define ClownCD_CHD_LzmaDec_Allocate LzmaDec_Allocate
#define ClownCD_CHD_LzmaDecode LzmaDecode

#include "real/LzmaDec.h"
