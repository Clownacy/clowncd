#ifndef CLOWNCD_DISC_CUE_H
#define CLOWNCD_DISC_CUE_H

#include "../disc.h"

cc_bool ClownCD_Disc_CueDetect(ClownCD_File *file);
void ClownCD_Disc_CueOpen(ClownCD_Disc *disc);
cc_bool ClownCD_Disc_CueSeekTrackIndex(ClownCD_Disc *disc, unsigned int track, unsigned int index);

#endif /* CLOWNCD_DISC_CUE_H */
