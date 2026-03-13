#ifndef CLOWNCD_NO_CHD
#define CLOWNCD_CHD
#endif

#include "libraries/clownresampler/clownresampler.c"
#include "source/audio.c"
#include "source/clowncd.c"
#include "source/cue.c"
#include "source/disc.c"
#include "source/error.c"
#include "source/file-io.c"
#include "source/utilities.c"
#include "source/audio/flac.c"
#include "source/audio/mp3.c"
#include "source/audio/vorbis.c"
#include "source/audio/wav.c"
#include "source/disc/clowncd.c"
#include "source/disc/cue.c"
#include "source/disc/raw.c"

#ifdef CLOWNCD_CHD
#include "libraries/chd/unity.c"
#include "source/disc/chd.c"
#endif
