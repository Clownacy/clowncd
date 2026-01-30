#ifndef CLOWNCD_DISC_CLOWNCD_H
#define CLOWNCD_DISC_CLOWNCD_H

#include "../disc.h"

cc_bool ClownCD_Disc_ClownCDDetect(ClownCD_File *file);
void ClownCD_Disc_ClownCDOpen(ClownCD_Disc *disc);
cc_bool ClownCD_Disc_ClownCDSeekTrackIndex(ClownCD_Disc *disc, unsigned int track, unsigned int index);

#endif /* CLOWNCD_DISC_CLOWNCD_H */
