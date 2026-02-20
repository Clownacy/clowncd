/* Use libchdr's copy of dr_flac. */
#ifdef CLOWNCD_CHD
#undef DR_FLAC_IMPLEMENTATION
#endif
#include "../../disc/chd/libchdr/include/dr_libs/dr_flac.h"
