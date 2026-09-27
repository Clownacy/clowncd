cmake_minimum_required(VERSION 3.7...3.12)

project(clowncd-chd LANGUAGES C)

add_library(clowncd-chd STATIC
	# libretro's convenience wrapper
	"streams/chd_stream.c"
	"streams/chd_stream.h"
	# libretro utilities
	"boolean.h"
)

find_package(PkgConfig)
if(PkgConfig_FOUND)
	pkg_check_modules(libchdr IMPORTED_TARGET libchdr>=0.3.0)
endif()

if(TARGET PkgConfig::libchdr)
	target_link_libraries(clowncd-chd PUBLIC PkgConfig::libchdr)
else()
	# Disable libchdr features that we do not need.
	set(CHDR_WANT_RAW_DATA_SECTOR OFF CACHE BOOL "" FORCE)
	set(CHDR_WANT_SUBCODE OFF CACHE BOOL "" FORCE)
	set(CHDR_VERIFY_BLOCK_CRC OFF CACHE BOOL "" FORCE)

	add_subdirectory("libchdr" EXCLUDE_FROM_ALL)
	target_link_libraries(clowncd-chd PUBLIC chdr-static)
endif()
