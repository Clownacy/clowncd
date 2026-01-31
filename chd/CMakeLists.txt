cmake_minimum_required(VERSION 3.0...3.12)

project(clowncd-chd LANGUAGES C)

add_library(clowncd-chd STATIC
	# libchdr
	"libretro-common/libchdr/bitstream.c"
	"libretro-common/libchdr/bitstream.h"
	"libretro-common/libchdr/cdrom.c"
	"libretro-common/libchdr/cdrom.h"
	"libretro-common/libchdr/chd.c"
	"libretro-common/libchdr/chd.h"
	"libretro-common/libchdr/chdconfig.h"
	"libretro-common/libchdr/coretypes.h"
	"libretro-common/libchdr/huffman.c"
	"libretro-common/libchdr/huffman.h"
	"libretro-common/libchdr/minmax.h"
	# libretro's convenience wrapper
	"libretro-common/streams/chd_stream.c"
	"libretro-common/streams/chd_stream.h"
	# libretro utilities
	"libretro-common/boolean.h"
	"libretro-common/retro_endianness.h"
	"libretro-common/retro_inline.h"
)

# zlib
target_compile_definitions(clowncd-chd PRIVATE HAVE_ZLIB)
target_sources(clowncd-chd PRIVATE
	"libretro-common/libchdr/zlib.c"
	"libretro-common/libchdr/zlib.h"
)

find_package(ZLIB)
if(ZLIB_FOUND)
	target_link_libraries(clowncd-chd PRIVATE ZLIB::ZLIB)
else()
	target_sources(clowncd-chd PRIVATE
		"libretro-common/libchdr/libraries/miniz-3.1.0/miniz.c"
		"libretro-common/libchdr/libraries/miniz-3.1.0/miniz.h"
		"libretro-common/libchdr/libraries/miniz-3.1.0/zlib.h"
	)
endif()

# 7zip
target_sources(clowncd-chd PRIVATE
	"libretro-common/libchdr/libraries/7zip/C/CpuArch.c"
	"libretro-common/libchdr/libraries/7zip/C/LzFind.c"
	"libretro-common/libchdr/libraries/7zip/C/LzFindMt.c"
	"libretro-common/libchdr/libraries/7zip/C/LzFindOpt.c"
	"libretro-common/libchdr/libraries/7zip/C/LzmaEnc.c"
	"libretro-common/libchdr/libraries/7zip/C/LzmaDec.c"
	"libretro-common/libchdr/libraries/7zip/C/Threads.c"
)
target_compile_definitions(clowncd-chd PRIVATE HAVE_7ZIP)
target_sources(clowncd-chd PRIVATE
	"libretro-common/libchdr/lzma.c"
	"libretro-common/libchdr/lzma.h"
)

# flac
target_compile_definitions(clowncd-chd PRIVATE HAVE_FLAC HAVE_DR_FLAC)
target_sources(clowncd-chd PRIVATE
	"libretro-common/libchdr/flac.c"
	"libretro-common/libchdr/flac.h"
	"libretro-common/libchdr/flac_codec.c"
)

# zstd
target_compile_definitions(clowncd-chd PRIVATE HAVE_ZSTD)
target_sources(clowncd-chd PRIVATE
	"libretro-common/libchdr/zstd.c"
	"libretro-common/libchdr/zstd.h"
)

find_package(zstd)
if(zstd_FOUND)
	target_link_libraries(clowncd-chd PRIVATE zstd)
else()
	target_sources(clowncd-chd PRIVATE
		"libretro-common/libchdr/libraries/zstd-1.5.7/zstd.h"
		"libretro-common/libchdr/libraries/zstd-1.5.7/zstd_errors.h"
		"libretro-common/libchdr/libraries/zstd-1.5.7/zstddeclib.c"
	)
endif()
