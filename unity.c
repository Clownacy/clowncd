#ifndef CLOWNCD_NO_CHD
#define CLOWNCD_CHD
#endif

#include "audio.c"
#include "clowncd.c"
#include "cue.c"
#include "disc.c"
#include "error.c"
#include "file-io.c"
#include "utilities.c"
#include "audio/flac.c"
#include "audio/mp3.c"
#include "audio/vorbis.c"
#include "audio/wav.c"
#include "audio/libraries/clownresampler/clownresampler.c"
#include "disc/clowncd.c"
#include "disc/cue.c"
#include "disc/raw.c"

#ifdef CLOWNCD_CHD
#include "disc/chd.c"
#include "disc/chd/unity.c"
#endif
