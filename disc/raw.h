#ifndef CLOWNCD_DISC_RAW_H
#define CLOWNCD_DISC_RAW_H

#include "../disc.h"

void ClownCD_Disc_RawOpen(ClownCD_Disc *disc);
cc_bool ClownCD_Disc_RawSeekTrackIndex(ClownCD_Disc *disc, unsigned int track, unsigned int index, cc_bool has_full_sized_sectors);

#endif /* CLOWNCD_DISC_RAW_H */
