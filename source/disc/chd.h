#ifndef CLOWNCD_DISC_CHD_H
#define CLOWNCD_DISC_CHD_H

#include "../disc.h"

cc_bool ClownCD_Disc_CHDDetect(ClownCD_File *file);
void ClownCD_Disc_CHDOpen(ClownCD_Disc *disc);
cc_bool ClownCD_Disc_CHDSeekTrackIndex(ClownCD_Disc *disc, unsigned int track, unsigned int index);

#endif /* CLOWNCD_DISC_CHD_H */
