cmake_minimum_required(VERSION 3.0...3.12)

project(clowncd-chd LANGUAGES C)

add_library(clowncd-chd STATIC
	# libretro's convenience wrapper
	"streams/chd_stream.c"
	"streams/chd_stream.h"
	# libretro utilities
	"boolean.h"
	"retro_inline.h"
)

add_subdirectory("libchdr" EXCLUDE_FROM_ALL)
target_link_libraries(clowncd-chd PUBLIC chdr-static)
