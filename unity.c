#define CLOWNCD_CHD

#define HAVE_ZLIB
#define HAVE_7ZIP
#define HAVE_FLAC
#define HAVE_DR_FLAC
#define HAVE_ZSTD

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

/* Disable unused features of miniz. */
#define MINIZ_NO_ARCHIVE_APIS
#define MINIZ_NO_ARCHIVE_WRITING_APIS
#define MINIZ_NO_DEFLATE_APIS
#define MINIZ_NO_STDIO
#define MINIZ_NO_TIME

#include "disc/chd.c"
#include "disc/chd/libchdr/libraries/lzma-25.01/LzmaDec.c"
#include "disc/chd/libchdr/libraries/miniz-3.1.0/miniz.c"
#include "disc/chd/libchdr/libraries/zstd-1.5.7/zstddeclib.c"
#include "disc/chd/libchdr/bitstream.c"
#include "disc/chd/libchdr/cdrom.c"
#include "disc/chd/libchdr/chd.c"
#include "disc/chd/libchdr/flac.c"
#include "disc/chd/libchdr/flac_codec.c"
#include "disc/chd/libchdr/huffman.c"
#include "disc/chd/libchdr/lzma.c"
#include "disc/chd/libchdr/zlib.c"
#include "disc/chd/libchdr/zstd.c"
#include "disc/chd/streams/chd_stream.c"
#endif