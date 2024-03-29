cmake_minimum_required(VERSION 3.0...3.12)

project(clowncd LANGUAGES C)

add_library(clowncd STATIC "clowncd.c" "clowncd.h")

add_executable(clowncd-test "test.c")

target_link_libraries(clowncd-test PRIVATE clowncd)

add_executable(clowncd-converter
	"cuebinconverter.c"
	"cue.c"
	"cue.h"
)
