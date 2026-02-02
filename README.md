# ClownCD

A portable library for reading Compact Disc images. Also included are some
conversion tools.


## Supported Formats

- BIN+CUE
- CHD
- ISO+FLAC
- ISO+MP3
- ISO+OGG
- ISO+WAV


## Building

A CMake script is provided, which uses system-library versions of libsndfile,
zlib, and zstd if they are available.

Alternatively, a `unity.c` file is provided which can be compiled or included
akin to a single-file library. This is useful for libretro cores, which use
Makefiles instead of CMake.


## Licence

ClownCD itself is licensed under the 0BSD licence, but it utilises several
libraries which have their own licensing:

| Library         | Licence       |
|-----------------|---------------|
| ClownCD         | 0BSD          |
| dr_flac         | Public Domain |
| dr_mp3          | Public Domain |
| dr_wav          | Public Domain |
| stb_vorbis      | Public Domain |
| libchdr         | BSD-3-Clause  |
| libretro-common | MIT           |
| LZMA SDK        | Public Domain |
| miniz           | MIT           |
| zstd            | BSD-3-Clause  |


## Utilities

`clowncd-cue-converter` converts a CUE file to a small binary header. Prefixing
this header to a raw Mega CD disc image (2352-bytes-per-sector BIN file) yields
a file which can be used with ClownMDEmu.

`clowncd-split` extracts a disc image to a CUE file, and assorted ISO and OGG
files.
