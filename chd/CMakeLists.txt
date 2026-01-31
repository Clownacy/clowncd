cmake_minimum_required(VERSION 3.11...3.12)

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

include(FetchContent)

# zlib
target_compile_definitions(clowncd-chd PRIVATE HAVE_ZLIB)
target_sources(clowncd-chd PRIVATE "libretro-common/formats/libchdr/libchdr_zlib.c")

find_package(ZLIB)
if(ZLIB_FOUND)
	target_link_libraries(clowncd-chd PRIVATE ZLIB::ZLIB)
else()
	FetchContent_Declare(
		zlib
		GIT_REPOSITORY https://github.com/madler/zlib.git
		GIT_TAG        51b7f2abdade71cd9bb0e7a373ef2610ec6f9daf # v1.3.1.2
	)
	FetchContent_MakeAvailable(zlib)
	target_link_libraries(clowncd-chd PRIVATE zlibstatic)
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

# zstd
target_compile_definitions(clowncd-chd PRIVATE HAVE_ZSTD)
target_sources(clowncd-chd PRIVATE "libretro-common/formats/libchdr/libchdr_zstd.c")

find_package(ZSTD)
if(ZSTD_FOUND)
	target_link_libraries(clowncd-chd PRIVATE ZSTD)
else()
	FetchContent_Declare(
		zstd
		GIT_REPOSITORY https://github.com/facebook/zstd.git
		GIT_TAG        f8745da6ff1ad1e7bab384bd1f9d742439278e99 # v1.5.7
		SOURCE_SUBDIR  build/cmake
	)
	FetchContent_MakeAvailable(zstd)
	target_link_libraries(clowncd-chd PRIVATE libzstd_static)
endif()
