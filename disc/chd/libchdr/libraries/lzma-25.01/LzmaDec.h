/* Namespace some symbols to avoid linker errors in static libretro builds. */
#define LzmaDec_InitDicAndState ClownCD_CHD_LzmaDec_InitDicAndState
#define LzmaDec_Init ClownCD_CHD_LzmaDec_Init
#define LzmaDec_DecodeToDic ClownCD_CHD_LzmaDec_DecodeToDic
#define LzmaDec_DecodeToBuf ClownCD_CHD_LzmaDec_DecodeToBuf
#define LzmaDec_FreeProbs ClownCD_CHD_LzmaDec_FreeProbs
#define LzmaDec_Free ClownCD_CHD_LzmaDec_Free
#define LzmaProps_Decode ClownCD_CHD_LzmaProps_Decode
#define LzmaDec_AllocateProbs ClownCD_CHD_LzmaDec_AllocateProbs
#define LzmaDec_Allocate ClownCD_CHD_LzmaDec_Allocate
#define LzmaDecode ClownCD_CHD_LzmaDecode

#include "real/LzmaDec.h"
