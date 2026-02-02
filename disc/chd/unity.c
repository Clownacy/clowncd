#define HAVE_ZLIB
#define HAVE_7ZIP
#define HAVE_FLAC
#define HAVE_DR_FLAC
#define HAVE_ZSTD

/* Disable unused features of miniz. */
#define MINIZ_NO_ARCHIVE_APIS
#define MINIZ_NO_DEFLATE_APIS
#define MINIZ_NO_STDIO
#define MINIZ_NO_TIME

#include "libchdr/libraries/lzma-25.01/LzmaDec.c"
#include "libchdr/libraries/miniz-3.1.0/miniz.c"
#include "libchdr/libraries/zstd-1.5.7/zstddeclib.c"
#include "libchdr/bitstream.c"
#include "libchdr/cdrom.c"
#include "libchdr/chd.c"
#include "libchdr/flac.c"
#include "libchdr/flac_codec.c"
#include "libchdr/huffman.c"
#include "libchdr/lzma.c"
#include "libchdr/zlib.c"
#include "libchdr/zstd.c"
#include "streams/chd_stream.c"
