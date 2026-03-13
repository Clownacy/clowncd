cmake_minimum_required(VERSION 3.0...3.12)

project(clowncd LANGUAGES C)

option(CLOWNCD_CHD "Support CHD files, leveraging various permissively-licensed libraries." ON)

add_library(clowncd STATIC
	"source/disc/clowncd.c"
	"source/disc/clowncd.h"
	"source/disc/chd.c"
	"source/disc/chd.h"
	"source/disc/cue.c"
	"source/disc/cue.h"
	"source/disc/raw.c"
	"source/disc/raw.h"
	"source/audio.c"
	"source/audio.h"
	"source/audio-common.h"
	"source/clowncd.c"
	"source/clowncd.h"
	"source/common.h"
	"source/cue.c"
	"source/cue.h"
	"source/disc.c"
	"source/disc.h"
	"source/error.c"
	"source/error.h"
	"source/file-io.c"
	"source/file-io.h"
	"source/utilities.c"
	"source/utilities.h"
)

if(CLOWNCD_CHD)
	add_subdirectory("libraries/chd" EXCLUDE_FROM_ALL)
	target_link_libraries(clowncd PRIVATE clowncd-chd)
	target_compile_definitions(clowncd PRIVATE CLOWNCD_CHD)
endif()

add_subdirectory("libraries/clownresampler" EXCLUDE_FROM_ALL)
target_link_libraries(clowncd PRIVATE clownresampler)

find_package(SndFile)

if(TARGET SndFile::sndfile)
	target_link_libraries(clowncd PRIVATE SndFile::sndfile)
	target_compile_definitions(clowncd PUBLIC CLOWNCD_LIBSNDFILE)
	target_sources(clowncd PRIVATE
		"source/audio/libsndfile.c"
		"source/audio/libsndfile.h"
	)
else()
	target_sources(clowncd PRIVATE
		"source/audio/flac.c"
		"source/audio/flac.h"
		"source/audio/mp3.c"
		"source/audio/mp3.h"
		"source/audio/vorbis.c"
		"source/audio/vorbis.h"
		"source/audio/wav.c"
		"source/audio/wav.h"
	)
endif()

if(TARGET SndFile::sndfile)
	add_executable(clowncd-split "source/split.c")
	target_link_libraries(clowncd-split PRIVATE clowncd)
	target_link_libraries(clowncd-split PRIVATE SndFile::sndfile)
endif()

add_executable(clowncd-cue-converter "source/cue-converter.c")
target_link_libraries(clowncd-cue-converter PRIVATE clowncd)
