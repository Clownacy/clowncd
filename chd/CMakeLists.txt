cmake_minimum_required(VERSION 3.0...3.12)

project(clowncd-chd LANGUAGES C)

add_library(clowncd-chd STATIC
	# libchdr
	"libchdr/bitstream.c"
	"libchdr/bitstream.h"
	"libchdr/cdrom.c"
	"libchdr/cdrom.h"
	"libchdr/chd.c"
	"libchdr/chd.h"
	"libchdr/chdconfig.h"
	"libchdr/coretypes.h"
	"libchdr/huffman.c"
	"libchdr/huffman.h"
	"libchdr/minmax.h"
	# libretro's convenience wrapper
	"streams/chd_stream.c"
	"streams/chd_stream.h"
	# libretro utilities
	"boolean.h"
	"retro_inline.h"
)

# zlib
target_compile_definitions(clowncd-chd PRIVATE HAVE_ZLIB)
target_sources(clowncd-chd PRIVATE
	"libchdr/zlib.c"
	"libchdr/zlib.h"
)

find_package(ZLIB)
if(ZLIB_FOUND)
	target_compile_definitions(clowncd-chd PRIVATE CLOWNCD_CHD_USE_SYSTEM_ZLIB)
	target_link_libraries(clowncd-chd PRIVATE ZLIB::ZLIB)
else()
	target_sources(clowncd-chd PRIVATE
		"libchdr/libraries/miniz-3.1.0/miniz.c"
		"libchdr/libraries/miniz-3.1.0/miniz.h"
		"libchdr/libraries/miniz-3.1.0/zlib.h"
	)
endif()

# 7zip
target_sources(clowncd-chd PRIVATE
	"libchdr/libraries/7zip/C/CpuArch.c"
	"libchdr/libraries/7zip/C/LzFind.c"
	"libchdr/libraries/7zip/C/LzFindMt.c"
	"libchdr/libraries/7zip/C/LzFindOpt.c"
	"libchdr/libraries/7zip/C/LzmaEnc.c"
	"libchdr/libraries/7zip/C/LzmaDec.c"
	"libchdr/libraries/7zip/C/Threads.c"
)
target_compile_definitions(clowncd-chd PRIVATE HAVE_7ZIP)
target_sources(clowncd-chd PRIVATE
	"libchdr/lzma.c"
	"libchdr/lzma.h"
)

# flac
target_compile_definitions(clowncd-chd PRIVATE HAVE_FLAC HAVE_DR_FLAC)
target_sources(clowncd-chd PRIVATE
	"libchdr/flac.c"
	"libchdr/flac.h"
	"libchdr/flac_codec.c"
)

# zstd
target_compile_definitions(clowncd-chd PRIVATE HAVE_ZSTD)
target_sources(clowncd-chd PRIVATE
	"libchdr/zstd.c"
	"libchdr/zstd.h"
)

find_package(zstd)
if(zstd_FOUND)
	target_compile_definitions(clowncd-chd PRIVATE CLOWNCD_CHD_USE_SYSTEM_ZSTD)
	target_link_libraries(clowncd-chd PRIVATE zstd)
else()
	target_sources(clowncd-chd PRIVATE
		"libchdr/libraries/zstd-1.5.7/zstd.h"
		"libchdr/libraries/zstd-1.5.7/zstd_errors.h"
		"libchdr/libraries/zstd-1.5.7/zstddeclib.c"
	)
endif()
