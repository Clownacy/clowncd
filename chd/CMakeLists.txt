cmake_minimum_required(VERSION 3.0...3.12)

project(clowncd-chd LANGUAGES C)

add_library(clowncd-chd STATIC
	"libretro-common/libchdr/bitstream.c"
	"libretro-common/libchdr/cdrom.c"
	"libretro-common/libchdr/chd.c"
	"libretro-common/libchdr/huffman.c"
	"libretro-common/streams/chd_stream.c"
	"libretro-common/streams/chd_stream.h"
)

# zlib
target_compile_definitions(clowncd-chd PRIVATE HAVE_ZLIB)
target_sources(clowncd-chd PRIVATE "libretro-common/libchdr/zlib.c")

find_package(ZLIB)
if(ZLIB_FOUND)
	target_link_libraries(clowncd-chd PRIVATE ZLIB::ZLIB)
else()
	target_include_directories(clowncd-chd PRIVATE "miniz-3.1.0")
	target_sources(clowncd-chd PRIVATE
		"miniz-3.1.0/miniz.c"
		"miniz-3.1.0/miniz.h"
		"miniz-3.1.0/zlib.h"
	)
endif()

# 7zip
target_include_directories(clowncd-chd PRIVATE "7zip/C")
target_sources(clowncd-chd PRIVATE
	"7zip/C/CpuArch.c"
	"7zip/C/LzFind.c"
	"7zip/C/LzFindMt.c"
	"7zip/C/LzFindOpt.c"
	"7zip/C/LzmaEnc.c"
	"7zip/C/LzmaDec.c"
	"7zip/C/Threads.c"
)
target_compile_definitions(clowncd-chd PRIVATE HAVE_7ZIP)
target_sources(clowncd-chd PRIVATE "libretro-common/libchdr/lzma.c")

# flac
target_compile_definitions(clowncd-chd PRIVATE HAVE_FLAC HAVE_DR_FLAC)
target_sources(clowncd-chd PRIVATE
	"libretro-common/libchdr/flac.c"
	"libretro-common/libchdr/flac_codec.c"
)
target_include_directories(clowncd-chd PRIVATE "include")

# zstd
target_compile_definitions(clowncd-chd PRIVATE HAVE_ZSTD)
target_sources(clowncd-chd PRIVATE "libretro-common/libchdr/zstd.c")

find_package(zstd)
if(zstd_FOUND)
	target_link_libraries(clowncd-chd PRIVATE zstd)
else()
	target_include_directories(clowncd-chd PRIVATE "zstd-1.5.7")
	target_sources(clowncd-chd PRIVATE
		"zstd-1.5.7/zstd.h"
		"zstd-1.5.7/zstd_errors.h"
		"zstd-1.5.7/zstddeclib.c"
	)
endif()
