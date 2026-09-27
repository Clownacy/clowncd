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

	# Avoid vendored libraries when possible.
	find_package(ZLIB)
	if(ZLIB_FOUND)
		set(WITH_SYSTEM_ZLIB ON CACHE BOOL "" FORCE)
	else()
		set(WITH_SYSTEM_ZLIB OFF CACHE BOOL "" FORCE)
	endif()
	find_package(zstd)
	if(zstd_FOUND)
		set(WITH_SYSTEM_ZSTD ON CACHE BOOL "" FORCE)
	else()
		set(WITH_SYSTEM_ZSTD OFF CACHE BOOL "" FORCE)
	endif()

	add_subdirectory("libchdr" EXCLUDE_FROM_ALL)
	target_link_libraries(clowncd-chd PUBLIC chdr-static)
endif()
