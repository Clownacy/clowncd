cmake_minimum_required(VERSION 3.0...3.12)

project(clowncd-chd LANGUAGES C)

add_library(clowncd-chd STATIC
	# libretro's convenience wrapper
	"streams/chd_stream.c"
	"streams/chd_stream.h"
	# libretro utilities
	"boolean.h"
)

# Disable libchdr features that we do not need.
set(CHDR_WANT_RAW_DATA_SECTOR OFF)
set(CHDR_WANT_SUBCODE OFF)
set(CHDR_VERIFY_BLOCK_CRC OFF)

add_subdirectory("libchdr" EXCLUDE_FROM_ALL)
target_link_libraries(clowncd-chd PUBLIC chdr-static)
