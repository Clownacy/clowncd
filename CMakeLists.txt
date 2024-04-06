cmake_minimum_required(VERSION 3.0...3.12)

project(clowncd LANGUAGES C)

add_library(clowncd STATIC
	"clowncd.c"
	"clowncd.h"
	"cue.c"
	"cue.h"
	"file-io.c"
	"file-io.h"
	"utilities.c"
	"utilities.h"
	"wav.c"
	"wav.h"
)

add_executable(clowncd-test "test.c")

target_link_libraries(clowncd-test PRIVATE clowncd)

find_package(SndFile)
target_link_libraries(clowncd-test PRIVATE SndFile::sndfile)

add_executable(clowncd-cue-converter
	"cue-converter.c"
)

target_link_libraries(clowncd-cue-converter PRIVATE clowncd)
