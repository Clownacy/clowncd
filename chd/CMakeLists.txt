cmake_minimum_required(VERSION 3.0...3.12)

project(clowncd-chd LANGUAGES C)

add_library(clowncd-chd STATIC
	"libretro-common/compat/compat_strl.c"
	"libretro-common/formats/libchdr/libchdr_bitstream.c"
	"libretro-common/formats/libchdr/libchdr_cdrom.c"
	"libretro-common/formats/libchdr/libchdr_chd.c"
	"libretro-common/formats/libchdr/libchdr_huffman.c"
	"libretro-common/streams/chd_stream.c"
	"libretro-common/include/streams/chd_stream.h"
)

target_include_directories(clowncd-chd PUBLIC "libretro-common/include")

# zlib
target_compile_definitions(clowncd-chd PRIVATE HAVE_ZLIB)
target_sources(clowncd-chd PRIVATE "libretro-common/formats/libchdr/libchdr_zlib.c")

find_package(ZLIB)
if(ZLIB_FOUND)
	target_link_libraries(clowncd-chd PRIVATE ZLIB::ZLIB)
else()
	add_subdirectory(zlib EXCLUDE_FROM_ALL)
	target_link_libraries(clowncd-chd PRIVATE ZLIB::ZLIBSTATIC)	
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
target_sources(clowncd-chd PRIVATE "libretro-common/formats/libchdr/libchdr_lzma.c")

# flac
target_compile_definitions(clowncd-chd PRIVATE HAVE_FLAC HAVE_DR_FLAC)
target_sources(clowncd-chd PRIVATE
	"libretro-common/formats/libchdr/libchdr_flac.c"
	"libretro-common/formats/libchdr/libchdr_flac_codec.c"
)
target_include_directories(clowncd-chd PRIVATE "include")
