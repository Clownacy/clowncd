cmake_minimum_required(VERSION 3.0...3.12)

project(clowncd LANGUAGES C)

add_library(clowncd STATIC
	"audio.c"
	"audio.h"
	"clowncd.c"
	"clowncd.h"
	"cue.c"
	"cue.h"
	"file-io.c"
	"file-io.h"
	"utilities.c"
	"utilities.h"
)

find_package(SndFile)

if(TARGET SndFile::sndfile)
	target_link_libraries(clowncd PRIVATE SndFile::sndfile)
	target_compile_definitions(clowncd PRIVATE CLOWNCD_LIBSNDFILE)
	target_sources(clowncd PRIVATE
		"audio/libsndfile.c"
		"audio/libsndfile.h"
	)
else()
	target_sources(clowncd PRIVATE
		"audio/flac.c"
		"audio/flac.h"
		"audio/mp3.c"
		"audio/mp3.h"
		"audio/vorbis.c"
		"audio/vorbis.h"
		"audio/wav.c"
		"audio/wav.h"
	)
endif()

if(TARGET SndFile::sndfile)
	add_executable(clowncd-split "split.c")
	target_link_libraries(clowncd-split PRIVATE clowncd)
	target_link_libraries(clowncd-split PRIVATE SndFile::sndfile)
endif()

add_executable(clowncd-cue-converter "cue-converter.c")
target_link_libraries(clowncd-cue-converter PRIVATE clowncd)
