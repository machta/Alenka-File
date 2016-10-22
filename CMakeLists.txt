cmake_minimum_required(VERSION 3.0.0)
project (Alenka-File)

option (BUILD_TEST "Builds unit tests." OFF)

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-Wall -pedantic")

if (BUILD_TEST)
	add_subdirectory (unit-test)
endif (BUILD_TEST)

include_directories (include)

file (GLOB SRC src/*)

add_library (alenka-file ${SRC})
